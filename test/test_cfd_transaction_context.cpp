#include "gtest/gtest.h"
#include <vector>

#include "cfdcore/cfdcore_amount.h"
#include "cfdcore/cfdcore_bytedata.h"
#include "cfdcore/cfdcore_coin.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_key.h"
#include "cfdcore/cfdcore_transaction_common.h"
#include "cfd/cfd_transaction.h"

using cfd::core::Amount;
using cfd::core::ByteData;
using cfd::core::ByteData256;
using cfd::core::CfdException;
using cfd::core::Privkey;
using cfd::core::Pubkey;
using cfd::core::Script;
using cfd::core::ScriptBuilder;
using cfd::core::ScriptOperator;
using cfd::core::SigHashAlgorithm;
using cfd::core::SigHashType;
using cfd::core::SignatureUtil;
using cfd::core::Txid;
using cfd::core::WitnessVersion;
using cfd::TransactionContext;

TEST(TransactionContext, CreateP2wpkhSignatureHash_Test) {
  // input-only transaction
  std::string tx = "0200000001efcdab89674523010000000000000000000000000000000000000000000000000000000000ffffffff01406f4001000000001976a9144b8fe2da0c979ec6027dc2287e65569f41d483ec88ac00000000";
  Txid txid("0000000000000000000000000000000000000000000000000123456789abcdef");
  uint32_t vout = 0;
  Pubkey pubkey("023e5e7a4a435f526a8b34d54c7355c8e57392b591b5a189ec88731953c568f8da");
  SigHashType sighash_type(SigHashAlgorithm::kSigHashAll);
  Amount amount = Amount::CreateBySatoshiAmount(21000000);

  std::string expect_sighash = "a3f0fa9b3e727bb746509b40a7cac316bf36685d55bebd6c7271d6e3a976f12d";
  
  TransactionContext txc(tx);
  ByteData sighash;
  EXPECT_NO_THROW(sighash = txc.CreateSignatureHash(txid, vout, pubkey, sighash_type, amount, WitnessVersion::kVersion0));
  
  EXPECT_STREQ(sighash.GetHex().c_str(), expect_sighash.c_str());
}

TEST(TransactionContext, CreateP2wshSignatureHash_Test) {
  // input-only transaction
  std::string tx = "0200000001efcdab89674523010000000000000000000000000000000000000000000000000000000000ffffffff01406f4001000000001976a9144b8fe2da0c979ec6027dc2287e65569f41d483ec88ac00000000";
  Txid txid("0000000000000000000000000000000000000000000000000123456789abcdef");
  uint32_t vout = 0;
  Script witness_script("0020aa7566645d0bce0682dca7086e103f00147bb3f4fb4e3f2e6113cb6f485e6789");
  SigHashType sighash_type(SigHashAlgorithm::kSigHashAll);
  Amount amount = Amount::CreateBySatoshiAmount(21000000);

  std::string expect_sighash = "4a9fc1813c51bdf4f14020c7ae570b22d9005da65a37cbf9cf10b79712a8dcc9";
  
  TransactionContext txc(tx);
  ByteData sighash;
  EXPECT_NO_THROW(sighash = txc.CreateSignatureHash(txid, vout, witness_script, sighash_type, amount, WitnessVersion::kVersion0));
  
  EXPECT_STREQ(sighash.GetHex().c_str(), expect_sighash.c_str());
}

TEST(TransactionContext, VerifyInputSignature_TEST_PKH) {
  // input-only transaction
  std::string tx = "0200000001efcdab89674523010000000000000000000000000000000000000000000000000000000000ffffffff01406f4001000000001976a9144b8fe2da0c979ec6027dc2287e65569f41d483ec88ac00000000";
  TransactionContext txc(tx);
  Privkey privkey = Privkey::GenerageRandomKey();

  // create sighash
  Txid txid("0000000000000000000000000000000000000000000000000123456789abcdef");
  uint32_t vout = 0;
  Pubkey pubkey = privkey.GeneratePubkey();
  SigHashType sighash_type(SigHashAlgorithm::kSigHashAll);
  ByteData sighash;
  EXPECT_NO_THROW(sighash = txc.CreateSignatureHash(txid, vout, pubkey, 
      sighash_type));

  ByteData signature;
  EXPECT_NO_THROW(signature = SignatureUtil::CalculateEcSignature(
      ByteData256(sighash.GetBytes()), privkey));

  EXPECT_TRUE(txc.VerifyInputSignature(signature, pubkey, txid, vout,
      sighash_type));

  // check signature another pubkey
  Privkey dummy;
  while(privkey.GetHex() == (dummy = Privkey::GenerageRandomKey()).GetHex()) {
    // do nothing
  }
  EXPECT_FALSE(txc.VerifyInputSignature(signature, dummy.GeneratePubkey(), txid, vout, sighash_type));
}

TEST(TransactionContext, VerifyInputSignature_TEST_SH) {
  // input-only transaction
  std::string tx = "0200000001efcdab89674523010000000000000000000000000000000000000000000000000000000000ffffffff01406f4001000000001976a9144b8fe2da0c979ec6027dc2287e65569f41d483ec88ac00000000";
  TransactionContext txc(tx);
  Privkey privkey = Privkey::GenerageRandomKey();
  ScriptBuilder sb = ScriptBuilder();

  // create sighash
  Txid txid("0000000000000000000000000000000000000000000000000123456789abcdef");
  uint32_t vout = 0;
  // create 1-of-2 multisig script
  Pubkey pubkey = privkey.GeneratePubkey();
  Script redeem_script = sb
      .AppendOperator(ScriptOperator::OP_1)
      .AppendData(pubkey)
      .AppendData(Pubkey("0229ebd1cac7855ca60b0846bd179ff3d411f807f3f3a43abf498e0a415c94d622"))
      .AppendOperator(ScriptOperator::OP_2)
      .AppendOperator(ScriptOperator::OP_CHECKMULTISIG)
      .Build();
  SigHashType sighash_type(SigHashAlgorithm::kSigHashAll);
  ByteData sighash;
  EXPECT_NO_THROW(sighash = txc.CreateSignatureHash(txid, vout, redeem_script,
      sighash_type));

  ByteData signature;
  EXPECT_NO_THROW(signature = SignatureUtil::CalculateEcSignature(
      ByteData256(sighash.GetBytes()), privkey));

  EXPECT_TRUE(txc.VerifyInputSignature(signature, pubkey, txid, vout,
      redeem_script, sighash_type));
}

TEST(TransactionContext, VerifyInputSignature_TEST_WPKH) {
  // input-only transaction
  std::string tx = "0200000001efcdab89674523010000000000000000000000000000000000000000000000000000000000ffffffff01406f4001000000001976a9144b8fe2da0c979ec6027dc2287e65569f41d483ec88ac00000000";
  TransactionContext txc(tx);
  Privkey privkey = Privkey::GenerageRandomKey();

  // create sighash
  Txid txid("0000000000000000000000000000000000000000000000000123456789abcdef");
  uint32_t vout = 0;
  Pubkey pubkey = privkey.GeneratePubkey();
  SigHashType sighash_type(SigHashAlgorithm::kSigHashAll);
  Amount amount = Amount::CreateBySatoshiAmount(21000000);
  ByteData sighash;
  EXPECT_NO_THROW(sighash = txc.CreateSignatureHash(txid, vout, pubkey,
      sighash_type, amount, WitnessVersion::kVersion0));

  ByteData signature;
  EXPECT_NO_THROW(signature = SignatureUtil::CalculateEcSignature(
      ByteData256(sighash.GetBytes()), privkey));

  EXPECT_TRUE(txc.VerifyInputSignature(signature, pubkey, txid, vout,
      sighash_type, amount, WitnessVersion::kVersion0));
}

TEST(TransactionContext, VerifyInputSignature_TEST_WSH) {
  // input-only transaction
  std::string tx = "0200000001efcdab89674523010000000000000000000000000000000000000000000000000000000000ffffffff01406f4001000000001976a9144b8fe2da0c979ec6027dc2287e65569f41d483ec88ac00000000";
  TransactionContext txc(tx);
  Privkey privkey = Privkey::GenerageRandomKey();
  ScriptBuilder sb = ScriptBuilder();

  // create sighash
  Txid txid("0000000000000000000000000000000000000000000000000123456789abcdef");
  uint32_t vout = 0;
  // create 1-of-2 multisig script
  Pubkey pubkey = privkey.GeneratePubkey();
  Script redeem_script = sb
      .AppendOperator(ScriptOperator::OP_1)
      .AppendData(pubkey)
      .AppendData(Pubkey("0229ebd1cac7855ca60b0846bd179ff3d411f807f3f3a43abf498e0a415c94d622"))
      .AppendOperator(ScriptOperator::OP_2)
      .AppendOperator(ScriptOperator::OP_CHECKMULTISIG)
      .Build();
  SigHashType sighash_type(SigHashAlgorithm::kSigHashAll);
  Amount amount = Amount::CreateBySatoshiAmount(21000000);
  ByteData sighash;
  EXPECT_NO_THROW(sighash = txc.CreateSignatureHash(txid, vout, redeem_script,
      sighash_type, amount, WitnessVersion::kVersion0));

  ByteData signature;
  EXPECT_NO_THROW(signature = SignatureUtil::CalculateEcSignature(
      ByteData256(sighash.GetBytes()), privkey));

  EXPECT_TRUE(txc.VerifyInputSignature(signature, pubkey, txid, vout,
      redeem_script, sighash_type, amount, WitnessVersion::kVersion0));
}