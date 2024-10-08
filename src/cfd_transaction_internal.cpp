// Copyright 2020 CryptoGarage
/**
 * @file cfd_transaction_internal.cpp
 *
 * @brief implementation of transaction utility class.
 */
#include "cfd_transaction_internal.h"  // NOLINT

#include <algorithm>
#include <string>
#include <vector>

#include "cfd/cfd_address.h"
#include "cfd/cfd_elements_transaction.h"
#include "cfd/cfd_transaction.h"
#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_coin.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_key.h"
#include "cfdcore/cfdcore_logger.h"
#include "cfdcore/cfdcore_schnorrsig.h"
#include "cfdcore/cfdcore_script.h"
#include "cfdcore/cfdcore_taproot.h"
#include "cfdcore/cfdcore_transaction_common.h"
#include "cfdcore/cfdcore_util.h"

namespace cfd {

using cfd::core::AbstractTxIn;
using cfd::core::AddressType;
using cfd::core::ByteData;
using cfd::core::ByteData256;
using cfd::core::CfdError;
using cfd::core::CfdException;
using cfd::core::CryptoUtil;
using cfd::core::OutPoint;
using cfd::core::Privkey;
using cfd::core::Pubkey;
using cfd::core::SchnorrPubkey;
using cfd::core::SchnorrSignature;
using cfd::core::Script;
using cfd::core::ScriptBuilder;
using cfd::core::ScriptElement;
using cfd::core::ScriptOperator;
using cfd::core::ScriptUtil;
using cfd::core::ScriptWitness;
using cfd::core::SigHashType;
using cfd::core::SignatureUtil;
using cfd::core::TaprootScriptTree;
using cfd::core::TaprootUtil;
using cfd::core::Txid;
using cfd::core::logger::warn;

// -----------------------------------------------------------------------------
// TransactionContextUtil
// -----------------------------------------------------------------------------

WitnessVersion TransactionContextUtil::CheckSignWithPrivkeySimple(
    const OutPoint& outpoint, const Pubkey& pubkey, const Privkey& privkey,
    AddressType address_type) {
  if (!outpoint.IsValid()) {
    warn(CFD_LOG_SOURCE, "Failed to SignWithPrivkeySimple. Invalid outpoint.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError, "Invalid outpoint.");
  }
  if (!pubkey.IsValid()) {
    warn(CFD_LOG_SOURCE, "Failed to SignWithPrivkeySimple. Invalid pubkey.");
    throw CfdException(CfdError::kCfdIllegalArgumentError, "Invalid pubkey.");
  }
  if (privkey.IsInvalid()) {
    warn(CFD_LOG_SOURCE, "Failed to SignWithPrivkeySimple. Invalid privkey.");
    throw CfdException(CfdError::kCfdIllegalArgumentError, "Invalid privkey.");
  }

  WitnessVersion version = WitnessVersion::kVersionNone;
  switch (address_type) {
    case AddressType::kP2wshAddress:
    case AddressType::kP2shP2wshAddress:
    case AddressType::kP2wpkhAddress:
    case AddressType::kP2shP2wpkhAddress:
      version = WitnessVersion::kVersion0;
      break;
    case AddressType::kTaprootAddress:
      version = WitnessVersion::kVersion1;
      break;
    default:
      break;
  }
  return version;
}

template <class Tx>
void TransactionContextUtil::AddPubkeyHashSign(
    Tx* transaction, const OutPoint& outpoint, const SignParameter& signature,
    const Pubkey& pubkey, AddressType address_type) {
  if ((signature.GetDataType() != SignDataType::kSign) &&
      (signature.GetDataType() != SignDataType::kBinary)) {
    warn(
        CFD_LOG_SOURCE,
        "Failed to AddPubkeyHashSign. Invalid signature type: {}",
        signature.GetDataType());
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Invalid signature type. signature type must be \"kSign\" "
        "or \"kBinary\".");  // NOLINT
  }
  if (!pubkey.IsValid()) {
    warn(CFD_LOG_SOURCE, "Failed to AddPubkeyHashSign. Invalid pubkey.");
    throw CfdException(CfdError::kCfdIllegalArgumentError, "Invalid pubkey.");
  }
  if (!outpoint.IsValid()) {
    warn(CFD_LOG_SOURCE, "Failed to AddPubkeyHashSign. Invalid outpoint.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError, "Invalid outpoint.");
  }

  bool has_witness = false;
  bool has_scriptsig = false;
  Script locking_script;
  std::vector<SignParameter> sign_params;
  switch (address_type) {
    case AddressType::kP2wpkhAddress:
      has_witness = true;
      break;
    case AddressType::kP2shP2wpkhAddress:
      has_witness = true;
      has_scriptsig = true;
      locking_script = ScriptUtil::CreateP2wpkhLockingScript(pubkey);
      break;
    case AddressType::kP2pkhAddress:
      has_scriptsig = true;
      break;
    default:
      warn(
          CFD_LOG_SOURCE,
          "Failed to AddPubkeyHashSign. Invalid address_type: {}",
          address_type);
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Invalid address_type. address_type must be \"p2wpkh\" "
          "or \"p2sh-p2wpkh\" or \"p2pkh\".");  // NOLINT
  }
  sign_params.push_back(signature);
  sign_params.push_back(SignParameter(pubkey));

  if (has_witness) {
    AddSign(transaction, outpoint, sign_params, true, true);
  }
  if (has_scriptsig) {
    if (!locking_script.IsEmpty()) {  // p2sh-p2wpkh
      std::vector<SignParameter> scriptsig;
      scriptsig.push_back(SignParameter(locking_script));
      AddSign(transaction, outpoint, scriptsig, false, true);
    } else {
      AddSign(transaction, outpoint, sign_params, false, true);
    }
  }
}

template <class Tx>
void TransactionContextUtil::AddScriptHashSign(
    Tx* transaction, const OutPoint& outpoint,
    const std::vector<SignParameter>& signatures, const Script& redeem_script,
    AddressType address_type, bool is_multisig_script) {
  if (redeem_script.IsEmpty()) {
    warn(CFD_LOG_SOURCE, "Failed to AddScriptHashSign. Empty script.");
    throw CfdException(CfdError::kCfdIllegalArgumentError, "Empty script.");
  }
  if (!outpoint.IsValid()) {
    warn(CFD_LOG_SOURCE, "Failed to AddScriptHashSign. Invalid outpoint.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError, "Invalid outpoint.");
  }
  for (const auto& signature : signatures) {
    if ((signature.GetDataType() != SignDataType::kSign) &&
        (signature.GetDataType() != SignDataType::kBinary)) {
      warn(
          CFD_LOG_SOURCE,
          "Failed to AddScriptHashSign. Invalid signature type: {}",
          signature.GetDataType());
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Invalid signature type. signature type must be \"kSign\" "
          "or \"kBinary\".");  // NOLINT
    }
  }

  bool has_witness = false;
  bool has_scriptsig = false;
  Script locking_script;
  std::vector<SignParameter> sign_params;
  switch (address_type) {
    case AddressType::kP2wshAddress:
      has_witness = true;
      break;
    case AddressType::kP2shP2wshAddress:
      has_witness = true;
      has_scriptsig = true;
      locking_script = ScriptUtil::CreateP2wshLockingScript(redeem_script);
      break;
    case AddressType::kP2shAddress:
      has_scriptsig = true;
      break;
    default:
      warn(
          CFD_LOG_SOURCE,
          "Failed to AddScriptHashSign. Invalid address_type: {}",
          address_type);
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Invalid address_type. address_type must be \"p2wsh\" "
          "or \"p2sh-p2wsh\" or \"p2sh\".");  // NOLINT
  }
  if (is_multisig_script) {
    if (has_witness) {
      sign_params.push_back(SignParameter(ByteData()));
    } else {
      sign_params.push_back(SignParameter(ScriptOperator::OP_0));
    }
  }
  for (const auto& signature : signatures) {
    sign_params.push_back(signature);
  }
  bool clear_stack = (sign_params.empty()) ? false : true;
  sign_params.push_back(SignParameter(redeem_script));

  if (has_witness) {
    AddSign(transaction, outpoint, sign_params, true, clear_stack);
  }
  if (has_scriptsig) {
    if (!locking_script.IsEmpty()) {  // p2sh-p2wpkh
      std::vector<SignParameter> scriptsig;
      scriptsig.push_back(SignParameter(locking_script));
      AddSign(transaction, outpoint, scriptsig, false, true);
    } else {
      AddSign(transaction, outpoint, sign_params, false, clear_stack);
    }
  }
}

template <class Tx>
void TransactionContextUtil::AddSign(
    Tx* transaction, const OutPoint& outpoint,
    const std::vector<SignParameter>& sign_params, bool insert_witness,
    bool clear_stack) {
  if (transaction == nullptr) {
    throw CfdException(
        CfdError::kCfdIllegalArgumentError, "Empty transaction.");
  }
  if (sign_params.empty()) {
    warn(CFD_LOG_SOURCE, "Failed to AddSign. Empty sign_params.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError, "Empty sign_params.");
  }
  if (!outpoint.IsValid()) {
    warn(CFD_LOG_SOURCE, "Failed to AddSign. Invalid outpoint.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError, "Invalid outpoint.");
  }

  bool has_op_code = false;
  if (!insert_witness) {
    for (const SignParameter& sign_param : sign_params) {
      if (sign_param.IsOpCode()) {
        ScriptOperator op_code = sign_param.GetOpCode();
        if (op_code.IsPushOperator()) {
          has_op_code = true;
        }
      }
    }
  }
  Txid txid = outpoint.GetTxid();
  uint32_t vout = outpoint.GetVout();
  uint32_t txin_index = transaction->GetTxInIndex(txid, vout);

  if (insert_witness) {
    if (clear_stack) {
      transaction->RemoveScriptWitnessStackAll(txin_index);
    }
    for (const SignParameter& sign_param : sign_params) {
      transaction->AddScriptWitnessStack(
          txin_index, sign_param.ConvertToSignature());
    }
  } else {
    Script script;
    if (!clear_stack) {
      script = transaction->GetTxIn(txin_index).GetUnlockingScript();
    }
    ScriptBuilder builder;
    for (const auto& element : script.GetElementList()) {
      builder.AppendElement(element);
    }
    for (const SignParameter& sign_param : sign_params) {
      if (has_op_code && sign_param.IsOpCode()) {
        // Checking push-operator is performed at the time of registration.
        builder.AppendOperator(sign_param.GetOpCode());
      } else {
        builder.AppendData(sign_param.ConvertToSignature());
      }
    }
    transaction->SetUnlockingScript(txin_index, builder.Build());
  }
}

template <class Tx>
void TransactionContextUtil::Verify(
    const Tx* transaction, const OutPoint& outpoint, const UtxoData& utxo,
    const AbstractTxIn* txin,
    std::function<ByteData256(
        const Tx*, const OutPoint&, const UtxoData&, const SigHashType&,
        const Pubkey&, const Script&, WitnessVersion, const ByteData*,
        const TaprootScriptTree*)>
        create_sighash_func) {
  if (TransactionContextUtil::IsOpTrueLockingScript(utxo, txin)) {
    return;  // OP_TRUE only.
  }

  // Correspondence when addressType is extracted from address string.
  UtxoData work_utxo = utxo;
  if (work_utxo.address_type == AddressType::kP2shAddress) {
    std::vector<ScriptElement> items;
    items = txin->GetUnlockingScript().GetElementList();
    if (items.size() == 1) {
      Script script = Script(items[0].GetBinaryData());
      if (script.IsP2wpkhScript()) {
        work_utxo.address_type = AddressType::kP2shP2wpkhAddress;
      } else if (script.IsP2wshScript()) {
        work_utxo.address_type = AddressType::kP2shP2wshAddress;
      }
    }
  }

  bool has_pubkey = false;
  WitnessVersion version = WitnessVersion::kVersionNone;
  std::vector<ByteData> signature_stack =
      TransactionContextUtil::GetVerifySignatureStack(
          work_utxo, txin, &version, &has_pubkey);

  if (version == WitnessVersion::kVersion1) {
    SchnorrPubkey witness_program(
        utxo.locking_script.GetElementList()[1].GetBinaryData());
    SchnorrSignature schnorr_sig;
    bool has_parity = false;
    uint8_t leaf_version = 0;
    SchnorrPubkey internal_pubkey;
    std::vector<ByteData256> nodes;
    Script tapscript;
    ByteData annex;
    TaprootUtil::ParseTaprootSignData(
        signature_stack, &schnorr_sig, &has_parity, &leaf_version,
        &internal_pubkey, &nodes, &tapscript, nullptr, &annex);
    if (has_pubkey) {
      auto sighash = create_sighash_func(
          transaction, outpoint, work_utxo, schnorr_sig.GetSigHashType(),
          Pubkey(), Script(), version, &annex, nullptr);
      bool is_verify = witness_program.Verify(schnorr_sig, sighash);
      if (!is_verify) {
        throw CfdException(
            CfdError::kCfdIllegalStateError, "Verify signature fail.");
      }
    } else {
      TaprootUtil::VerifyTaprootCommitment(
          has_parity, leaf_version, witness_program, internal_pubkey, nodes,
          tapscript);
      // At the moment, there is no function to run Script,
      // so no further confirmation is possible.
      throw CfdException(
          CfdError::kCfdIllegalStateError,
          "The script analysis of tapscript is not supported.");
    }
  } else if (has_pubkey) {
    SigHashType sighashtype;
    ByteData signature =
        CryptoUtil::ConvertSignatureFromDer(signature_stack[0], &sighashtype);
    Pubkey pubkey(signature_stack[1]);
    auto sighash = create_sighash_func(
        transaction, outpoint, work_utxo, sighashtype, pubkey, Script(),
        version, nullptr, nullptr);
    bool is_verify =
        SignatureUtil::VerifyEcSignature(sighash, pubkey, signature);
    if (!is_verify) {
      throw CfdException(
          CfdError::kCfdIllegalStateError, "Verify signature fail.");
    }
  } else {
    // check script format
    Script redeem_script(signature_stack.back());
    if (!redeem_script.IsMultisigScript()) {
      // At the moment, there is no function to run Script,
      // so no further confirmation is possible.
      throw CfdException(
          CfdError::kCfdIllegalStateError,
          "Unsupported script format. verify support is multisig only.");
    }
    if (work_utxo.address_type == AddressType::kP2shAddress) {
      if (signature_stack[0].GetHex() != "00") {
        throw CfdException(
            CfdError::kCfdIllegalStateError,
            "Invalid multisig format. top stack is OP_0 only.");
      }
    } else {
      if (!signature_stack[0].IsEmpty()) {
        throw CfdException(
            CfdError::kCfdIllegalStateError,
            "Invalid multisig format. top byte is empty only.");
      }
    }
    uint32_t req_num = 0;
    std::vector<Pubkey> pubkeys =
        ScriptUtil::ExtractPubkeysFromMultisigScript(redeem_script, &req_num);

    uint32_t sig_size = static_cast<uint32_t>(signature_stack.size()) - 2;
    std::vector<ByteData> signatures(sig_size);
    std::vector<SigHashType> sighashtype_list(sig_size);
    if (sig_size < req_num) {
      throw CfdException(
          CfdError::kCfdIllegalStateError,
          "Signature count is less than multisig require count.");
    }

    uint32_t success_count = 0;
    std::string fail_indexes;
    SigHashType sighashtype;
    int prev_key_index = -1;
    for (size_t index = 0; index < sig_size; ++index) {
      bool is_exist = false;
      auto signature = CryptoUtil::ConvertSignatureFromDer(
          signature_stack[index + 1], &sighashtype);
      auto sighash = create_sighash_func(
          transaction, outpoint, work_utxo, sighashtype, Pubkey(),
          redeem_script, version, nullptr, nullptr);
      for (size_t key_index = 0; key_index < pubkeys.size(); ++key_index) {
        const auto& pubkey = pubkeys[key_index];
        is_exist =
            SignatureUtil::VerifyEcSignature(sighash, pubkey, signature);
        if (is_exist) {
          if (prev_key_index >= static_cast<int>(key_index)) {
            throw CfdException(
                CfdError::kCfdIllegalStateError,
                "Signature order is incorrect.");
          }
          prev_key_index = static_cast<int>(key_index);
          break;
        }
      }
      if (is_exist) {
        ++success_count;
      } else {
        if (!fail_indexes.empty()) fail_indexes += ",";
        fail_indexes += std::to_string(index);
      }
    }

    if (success_count != sig_size) {
      throw CfdException(
          CfdError::kCfdIllegalStateError,
          "Verify signature fail. fail index = " + fail_indexes);
    }
  }
}

bool TransactionContextUtil::IsOpTrueLockingScript(
    const UtxoData& utxo, const AbstractTxIn* txin) {
  std::vector<ScriptElement> locking_script_items =
      utxo.locking_script.GetElementList();
  if (locking_script_items.size() == 1) {
    if (locking_script_items[0].GetOpCode() == ScriptOperator::OP_TRUE) {
      if (txin->GetScriptWitnessStackNum() != 0) {
        throw CfdException(
            CfdError::kCfdIllegalStateError,
            "Exist witness stack. OP_TRUE is unused.");
      }
      if (!txin->GetUnlockingScript().IsEmpty()) {
        throw CfdException(
            CfdError::kCfdIllegalStateError,
            "Exist scriptsig. OP_TRUE is unused.");
      }
      return true;  // OP_TRUE only.
    }
  }
  return false;
}

std::vector<ByteData> TransactionContextUtil::GetVerifySignatureStack(
    const UtxoData& utxo, const AbstractTxIn* txin,
    WitnessVersion* witness_version, bool* is_pubkey_stack) {
  bool has_witness = false;
  bool has_unlocking_script = false;
  if ((utxo.address_type == AddressType::kP2pkhAddress) ||
      (utxo.address_type == AddressType::kP2shAddress)) {
    if (txin->GetScriptWitnessStackNum() != 0) {
      throw CfdException(
          CfdError::kCfdIllegalStateError,
          "Exist witness stack. address is non-witness.");
    }
    if (txin->GetUnlockingScript().IsEmpty()) {
      throw CfdException(
          CfdError::kCfdIllegalStateError,
          "NotFound scriptsig. p2pkh or p2sh need scriptsig.");
    }
    has_unlocking_script = true;
  } else {
    if (txin->GetScriptWitnessStackNum() == 0) {
      throw CfdException(
          CfdError::kCfdIllegalStateError,
          "NotFound witness stack. segwit need scriptsig.");
    }
    has_witness = true;

    if ((utxo.address_type == AddressType::kP2shP2wpkhAddress) ||
        (utxo.address_type == AddressType::kP2shP2wshAddress)) {
      if (txin->GetUnlockingScript().IsEmpty()) {
        throw CfdException(
            CfdError::kCfdIllegalStateError,
            "NotFound scriptsig. p2sh-address need scriptsig.");
      }
      has_unlocking_script = true;
    }
  }

  std::vector<ByteData> signature_stack;
  if (has_witness) {
    signature_stack = txin->GetScriptWitness().GetWitness();
  }
  Script unlocking_script;
  if (has_unlocking_script) {
    auto script = txin->GetUnlockingScript();
    std::vector<ScriptElement> items = script.GetElementList();
    if (!has_witness) {
      // check verify
      for (const auto& item : items) {
        if (item.IsBinary()) {
          signature_stack.push_back(item.GetBinaryData());
        } else {  // ignore size buffer.
          ScriptBuilder build;
          build.AppendElement(item);
          signature_stack.push_back(build.Build().GetData());
        }
      }
    } else if (utxo.address_type == AddressType::kTaprootAddress) {
      // do nothing
    } else {
      if (items.size() != 1) {
        throw CfdException(
            CfdError::kCfdIllegalStateError,
            "Unmatch scriptsig format. stack count fail.");
      }
      script = Script(items[0].GetBinaryData());  // get script byte data.
      if (utxo.address_type == AddressType::kP2shP2wpkhAddress) {
        if (!script.IsP2wpkhScript()) {
          throw CfdException(
              CfdError::kCfdIllegalStateError,
              "Unmatch scriptsig format. "
              "p2sh-p2wpkh address's scriptsig need p2wpkh locking script.");
        }
      } else {  // if (utxo.address_type == AddressType::kP2shP2wshAddress)
        if (!script.IsP2wshScript()) {
          throw CfdException(
              CfdError::kCfdIllegalStateError,
              "Unmatch scriptsig format. "
              "p2sh-p2wsh address's scriptsig need p2wsh locking script.");
        }
      }
      unlocking_script = script;
    }
  }

  WitnessVersion version =
      (has_witness) ? WitnessVersion::kVersion0 : WitnessVersion::kVersionNone;
  bool has_pubkey = false;
  Script locking_script;
  if ((utxo.address_type == AddressType::kP2wpkhAddress) ||
      (utxo.address_type == AddressType::kP2shP2wpkhAddress) ||
      (utxo.address_type == AddressType::kP2pkhAddress)) {
    if (signature_stack.size() != 2) {
      throw CfdException(
          CfdError::kCfdIllegalStateError,
          "signature stack unmatch. stack count is 2 only.");
    }
    has_pubkey = true;
    Pubkey pubkey(signature_stack.back());
    if (utxo.address_type == AddressType::kP2pkhAddress) {
      locking_script = ScriptUtil::CreateP2pkhLockingScript(pubkey);
    } else {
      locking_script = ScriptUtil::CreateP2wpkhLockingScript(pubkey);
      if (utxo.address_type == AddressType::kP2shP2wpkhAddress) {
        if (locking_script.GetHex() != unlocking_script.GetHex()) {
          throw CfdException(
              CfdError::kCfdIllegalStateError,
              "p2sh-p2wpkh scriptsig unmatch.");
        }
        locking_script = ScriptUtil::CreateP2shLockingScript(locking_script);
      }
    }
  } else if (utxo.address_type == AddressType::kTaprootAddress) {
    version = WitnessVersion::kVersion1;

    SchnorrSignature schnorr_sig;
    bool has_parity = false;
    uint8_t leaf_version = 0;
    SchnorrPubkey internal_pubkey;
    std::vector<ByteData256> nodes;
    Script tapscript;
    ByteData annex;

    TaprootUtil::ParseTaprootSignData(
        signature_stack, &schnorr_sig, &has_parity, &leaf_version,
        &internal_pubkey, &nodes, &tapscript, nullptr, &annex);
    if (internal_pubkey.IsValid()) {
      TaprootScriptTree tree(leaf_version, tapscript);
      for (const auto& node : nodes) tree.AddBranch(node);
      TaprootUtil::CreateTapScriptControl(
          internal_pubkey, tree, nullptr, &locking_script);
    } else {
      has_pubkey = true;
      locking_script = utxo.locking_script;  // for ignore check
    }
  } else {
    // check script format
    Script redeem_script(signature_stack.back());
    if (utxo.address_type == AddressType::kP2shAddress) {
      locking_script = ScriptUtil::CreateP2shLockingScript(redeem_script);
    } else {
      locking_script = ScriptUtil::CreateP2wshLockingScript(redeem_script);
      if (utxo.address_type == AddressType::kP2shP2wshAddress) {
        if (locking_script.GetHex() != unlocking_script.GetHex()) {
          throw CfdException(
              CfdError::kCfdIllegalStateError,
              "p2sh-p2wsh scriptsig unmatch.");
        }
        locking_script = ScriptUtil::CreateP2shLockingScript(locking_script);
      }
    }
  }
  if (!utxo.locking_script.IsEmpty()) {
    if (locking_script.GetHex() != utxo.locking_script.GetHex()) {
      throw CfdException(
          CfdError::kCfdIllegalStateError, "Unmatch locking script.");
    }
  }

  if (witness_version != nullptr) *witness_version = version;
  if (is_pubkey_stack != nullptr) *is_pubkey_stack = has_pubkey;
  return signature_stack;
}

// -----------------------------------------------------------------------------
// TransactionContextUtil implements TransactionContext
// -----------------------------------------------------------------------------
template void TransactionContextUtil::AddPubkeyHashSign<TransactionContext>(
    TransactionContext* transaction, const OutPoint& outpoint,
    const SignParameter& signature, const Pubkey& pubkey,
    AddressType address_type);

template void TransactionContextUtil::AddScriptHashSign<TransactionContext>(
    TransactionContext* transaction, const OutPoint& outpoint,
    const std::vector<SignParameter>& signatures, const Script& redeem_script,
    AddressType address_type, bool is_multisig_script);

template void TransactionContextUtil::AddSign<TransactionContext>(
    TransactionContext* transaction, const OutPoint& outpoint,
    const std::vector<SignParameter>& sign_params, bool insert_witness,
    bool clear_stack);

template void TransactionContextUtil::Verify<TransactionContext>(
    const TransactionContext* transaction, const OutPoint& outpoint,
    const UtxoData& utxo, const AbstractTxIn* txin,
    std::function<ByteData256(
        const TransactionContext*, const OutPoint&, const UtxoData&,
        const SigHashType&, const Pubkey&, const Script&, WitnessVersion,
        const ByteData*, const TaprootScriptTree*)>
        create_sighash_func);

template void TransactionContextUtil::Verify<Transaction>(
    const Transaction* transaction, const OutPoint& outpoint,
    const UtxoData& utxo, const AbstractTxIn* txin,
    std::function<ByteData256(
        const Transaction*, const OutPoint&, const UtxoData&,
        const SigHashType&, const Pubkey&, const Script&, WitnessVersion,
        const ByteData*, const TaprootScriptTree*)>
        create_sighash_func);

// -----------------------------------------------------------------------------
// TransactionContextUtil implements ConfidentialTransactionContext
// -----------------------------------------------------------------------------
#ifndef CFD_DISABLE_ELEMENTS
template void
TransactionContextUtil::AddPubkeyHashSign<ConfidentialTransactionContext>(
    ConfidentialTransactionContext* transaction, const OutPoint& outpoint,
    const SignParameter& signature, const Pubkey& pubkey,
    AddressType address_type);

template void
TransactionContextUtil::AddScriptHashSign<ConfidentialTransactionContext>(
    ConfidentialTransactionContext* transaction, const OutPoint& outpoint,
    const std::vector<SignParameter>& signatures, const Script& redeem_script,
    AddressType address_type, bool is_multisig_script);

template void TransactionContextUtil::AddSign<ConfidentialTransactionContext>(
    ConfidentialTransactionContext* transaction, const OutPoint& outpoint,
    const std::vector<SignParameter>& sign_params, bool insert_witness,
    bool clear_stack);

template void TransactionContextUtil::Verify<ConfidentialTransactionContext>(
    const ConfidentialTransactionContext* transaction,
    const OutPoint& outpoint, const UtxoData& utxo, const AbstractTxIn* txin,
    std::function<ByteData256(
        const ConfidentialTransactionContext*, const OutPoint&,
        const UtxoData&, const SigHashType&, const Pubkey&, const Script&,
        WitnessVersion, const ByteData*, const TaprootScriptTree*)>
        create_sighash_func);
#endif  // CFD_DISABLE_ELEMENTS

}  // namespace cfd
