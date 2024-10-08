#include "gtest/gtest.h"
#include <vector>

#include "cfd/cfd_common.h"
#include "cfd/cfd_psbt.h"
#include "cfd/cfd_utxo.h"
#include "cfd/cfd_transaction_common.h"
#include "cfdcore/cfdcore_amount.h"
#include "cfdcore/cfdcore_common.h"
#include "cfdcore/cfdcore_descriptor.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_hdwallet.h"
#include "cfdcore/cfdcore_psbt.h"
#include "cfdcore/cfdcore_script.h"
#include "cfdcore/cfdcore_amount.h"
#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_transaction.h"
#include "cfdcore/cfdcore_util.h"

using cfd::core::CfdException;
using cfd::core::Address;
using cfd::core::AddressType;
using cfd::core::Amount;
using cfd::core::Descriptor;
using cfd::core::Script;
using cfd::core::ScriptBuilder;
using cfd::core::ScriptOperator;
using cfd::core::ScriptHash;
using cfd::core::ScriptElement;
using cfd::core::ByteData;
using cfd::core::ByteData256;
using cfd::core::HDWallet;
using cfd::core::OutPoint;
using cfd::core::Privkey;
using cfd::core::Pubkey;
using cfd::core::NetType;
using cfd::core::Txid;
using cfd::core::TxIn;
using cfd::core::TxInReference;
using cfd::core::Transaction;
using cfd::core::TxOut;
using cfd::core::TxOutReference;
using cfd::core::CryptoUtil;
using cfd::core::SigHashType;
using cfd::core::WitnessVersion;
using cfd::core::ScriptUtil;
using cfd::core::KeyData;
using cfd::core::HashUtil;
using cfd::Psbt;
using cfd::UtxoData;
using cfd::CoinSelectionOption;
using cfd::TransactionContext;

static const std::string g_psbt_utxo_witness =
    "02000000000101f1993fe8e7189542ee4506258e170201be292703cd275acb09ece16672fd848b0000000017160014ac9ef80b27af1c9d95c1db5d761319322bc42fc5ffffffff02080410240100000016001409de2a0431cbb3444fc22cad9d9a0fd09639721000e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac975307870247304402201e07df721c3322419e8f36d07eeae4795975ba0d9d19630ca3cd3dc0d4967172022015428e7be06b6567501539050bd791a380f00bbddbc5097ea97ba7be4017114a0121024aef43b1d5ac7ba5014998d63ceac583959d1fdc66ea2699cd84eeaf82a2830600000000";
    // [0]: 
/*
  HDWallet wallet1 = HDWallet(ByteData(g_psbt_seed1));
  EXPECT_STREQ("tprv8ghSmFAFHkyY1BsSsnwxKsPMgh9zSAFd8vLTniA2d6sK25xpWpGd4qQ4EpGzYwfvDwR77r7bzmdGbmasLwMjceUWThXGU9vsYQSsxgzKY7T", wallet1.GeneratePrivkey(NetType::kTestnet, "44h/0h/0h").ToString().c_str());
  std::string out_path = "44h/0h/0h/0/1";
  std::string fee_path = "44h/0h/0h/1/1";
  std::string fee0_path = "44h/0h/0h/1/0";
  auto data3 = wallet1.GeneratePrivkeyData(NetType::kTestnet, out_path);
  auto data4 = wallet1.GeneratePrivkeyData(NetType::kTestnet, fee_path);
  auto data0 = wallet1.GeneratePrivkeyData(NetType::kTestnet, fee0_path);
  Address addr3 = Address(NetType::kTestnet, WitnessVersion::kVersion0, data3.GetPubkey());
  Address addr4 = Address(NetType::kTestnet, WitnessVersion::kVersion0, data4.GetPubkey());
  Address addr4sh = Address(NetType::kTestnet, addr4.GetLockingScript());
  Address addr0 = Address(NetType::kTestnet, WitnessVersion::kVersion0, data0.GetPubkey());
  Address addr0pkh = Address(NetType::kTestnet, data0.GetPubkey());
  Address addr0sh = Address(NetType::kTestnet, addr0.GetLockingScript());
  Transaction tx;
  tx.AddTxIn(Txid("8b84fd7266e1ec09cb5a27cd032729be0102178e250645ee429518e7e83f99f1"), 0, 0xffffffff);
  tx.AddTxOut(Amount(4899996680), addr3.GetLockingScript());
  tx.AddTxOut(Amount(100000000), addr4sh.GetLockingScript());
  Amount amt(5000000000);
  auto sighash = tx.GetSignatureHash(0, addr0pkh.GetLockingScript().GetData(), SigHashType(), amt, WitnessVersion::kVersion0);
  auto sk = data0.GetPrivkey();
  auto sig = sk.CalculateEcSignature(sighash);
  auto der = CryptoUtil::ConvertSignatureToDer(sig, SigHashType());
  tx.AddScriptWitnessStack(0, der);
  tx.AddScriptWitnessStack(0, data0.GetPubkey().GetData());
  tx.SetUnlockingScript(0, std::vector<ByteData>{addr0.GetLockingScript().GetData()});
  EXPECT_STREQ("", tx.GetHex().c_str());
 */

static const std::string g_psbt_utxo_legacy =
    "0200000001c6d2ea36e2e802b52ddac665dacbed2f831b5263459e1ca734f5c945d7515e40000000006a473044022011b96c7d2d0d2e8dcb37138e18acc460752965add9cb8787df5c349df0e2ae6602202e93af31b64f5166e5605819555dabec57be794300fadb37052f31dddea9905c012103e3d244a3967e0b87765fda86c5ff38885f74993953b9584388aef30b26af6aecffffffff017851cd1d000000001976a9148d20443a91969e3bca0e240cd0ffe4dc98c63de288ac00000000";
    // [0]: 03044f2b3045e62efa936c5169586798a2f241890e19a2d6bc4d7a535429501d87
/*
  HDWallet wallet2 = HDWallet(ByteData(g_psbt_seed2));
  EXPECT_STREQ("tprv8fbPrDdF7Cdde4fLncessNymvfuvREAzhoMmfs3XqCuLcNVB9didfgXgb5V1NrxkqF7ZKibuyib4n6bujk1L5NfgVYnZZCwuyDdT21JAquv", wallet2.GeneratePrivkey(NetType::kTestnet, "44h/0h/0h").ToString().c_str());
  std::string out_path = "44h/0h/0h/0/1";
  std::string fee_path = "44h/0h/0h/1/1";
  auto data3 = wallet2.GeneratePrivkeyData(NetType::kTestnet, out_path);
  auto data4 = wallet2.GeneratePrivkeyData(NetType::kTestnet, fee_path);
  Address addr3 = Address(NetType::kTestnet, data3.GetPubkey());
  Address addr4 = Address(NetType::kTestnet, data4.GetPubkey());
  Transaction tx;
  tx.AddTxIn(Txid("405e51d745c9f534a71c9e4563521b832fedcbda65c6da2db502e8e236ead2c6"), 0, 0xffffffff);
  tx.AddTxOut(Amount(499995000), addr3.GetLockingScript());
  auto sighash = tx.GetSignatureHash(0, addr4.GetLockingScript().GetData(), SigHashType());
  auto sk = data4.GetPrivkey();
  auto sig = sk.CalculateEcSignature(sighash);
  auto der = CryptoUtil::ConvertSignatureToDer(sig, SigHashType());
  tx.SetUnlockingScript(0, std::vector<ByteData>{der, data4.GetPubkey().GetData()});
  EXPECT_STREQ("", tx.GetHex().c_str());
 */

static const std::string g_psbt_utxo_sh_wsh_multi = "02000000000102f1993fe8e7189542ee4506258e170201be292703cd275acb09ece16672fd848b0100000017160014c1768dd714526732f54de6c8581a78e385d0900affffffffc6d2ea36e2e802b52ddac665dacbed2f831b5263459e1ca734f5c945d7515e4001000000171600141305fd9c86bb2cd2d671928388eccd2b1140aa5affffffff010858cd1d0000000017a914945fb50391a70637c1ffc5ab7fb65308c2f2317587024730440220032350deae8e8fc60a45800508b09f475ba6043b40ce87f8020a6cc532e401ab02205979d8ad394e19ef9881bb987f77dab465f1112a746ca17a509215c2dbfea93201210206d743464ae738be5ba6d07dd434bed3f24af32f9fad805ffb9a241ec56ea24802473044022051f8776ba287c7424b2d783ebde964e25bde5fec4728735b8b455c806daae16602207988cabc3475109e264a7d378873d0fcee78959c46facdafb099af4ca6eace94012103e241f36e5e17a599c465ec82f3520e6d7a0506cc862de06ff4ba26b255a2f62c00000000";
/*
  HDWallet wallet1 = HDWallet(ByteData(g_psbt_seed1));
  HDWallet wallet2 = HDWallet(ByteData(g_psbt_seed2));
  std::string in_path = "44h/0h/0h/0/10";
  std::string out_path = "44h/0h/0h/0/11";
  auto data11 = wallet1.GeneratePrivkeyData(NetType::kTestnet, in_path);
  auto data12 = wallet2.GeneratePrivkeyData(NetType::kTestnet, in_path);
  auto data21 = wallet1.GeneratePrivkeyData(NetType::kTestnet, out_path);
  auto data22 = wallet2.GeneratePrivkeyData(NetType::kTestnet, out_path);
  Address addr11 = Address(NetType::kTestnet, WitnessVersion::kVersion0, data11.GetPubkey());
  Address addr12 = Address(NetType::kTestnet, WitnessVersion::kVersion0, data12.GetPubkey());
  Address addr11pkh = Address(NetType::kTestnet, data11.GetPubkey());
  Address addr12pkh = Address(NetType::kTestnet, data12.GetPubkey());
  auto multisig = ScriptUtil::CreateMultisigRedeemScript(
      2, std::vector<Pubkey>{data21.GetPubkey(), data22.GetPubkey()});
  Address addr2 = Address(NetType::kTestnet, WitnessVersion::kVersion0, multisig);
  Address addr2sh = Address(NetType::kTestnet, addr2.GetLockingScript());
  Transaction tx;
  tx.AddTxIn(Txid("8b84fd7266e1ec09cb5a27cd032729be0102178e250645ee429518e7e83f99f1"), 1, 0xffffffff);
  tx.AddTxIn(Txid("405e51d745c9f534a71c9e4563521b832fedcbda65c6da2db502e8e236ead2c6"), 1, 0xffffffff);
  tx.AddTxOut(Amount(499996680), addr2sh.GetLockingScript());
  Amount amt(250000000);
  auto sighash1 = tx.GetSignatureHash(0, addr11pkh.GetLockingScript().GetData(), SigHashType(), amt, WitnessVersion::kVersion0);
  auto sig1 = data11.GetPrivkey().CalculateEcSignature(sighash1);
  auto der1 = CryptoUtil::ConvertSignatureToDer(sig1, SigHashType());
  tx.AddScriptWitnessStack(0, der1);
  tx.AddScriptWitnessStack(0, data11.GetPubkey().GetData());
  tx.SetUnlockingScript(0, std::vector<ByteData>{addr11.GetLockingScript().GetData()});
  auto sighash2 = tx.GetSignatureHash(1, addr12pkh.GetLockingScript().GetData(), SigHashType(), amt, WitnessVersion::kVersion0);
  auto sig2 = data12.GetPrivkey().CalculateEcSignature(sighash2);
  auto der2 = CryptoUtil::ConvertSignatureToDer(sig2, SigHashType());
  tx.AddScriptWitnessStack(1, der2);
  tx.AddScriptWitnessStack(1, data12.GetPubkey().GetData());
  tx.SetUnlockingScript(1, std::vector<ByteData>{addr12.GetLockingScript().GetData()});
  EXPECT_STREQ("", tx.GetHex().c_str());
 */

static const std::string g_psbt_seed1 = "8bc106907003ea0b55f3ed4ce2fcf9a198d8c43f07e6ade8aacc5c20c33db12e";
// 44'/0'/0': tprv8ghSmFAFHkyY1BsSsnwxKsPMgh9zSAFd8vLTniA2d6sK25xpWpGd4qQ4EpGzYwfvDwR77r7bzmdGbmasLwMjceUWThXGU9vsYQSsxgzKY7T
static const std::string g_psbt_seed2 = "d3e3539eafb6af1f0ae374ecffd33bed394f5eb2e39f8957be63c258ac32ca97";
// 44'/0'/0': tprv8fbPrDdF7Cdde4fLncessNymvfuvREAzhoMmfs3XqCuLcNVB9didfgXgb5V1NrxkqF7ZKibuyib4n6bujk1L5NfgVYnZZCwuyDdT21JAquv

static std::vector<UtxoData> CfdTestGenerateUtxo(
    NetType net_type, const HDWallet& wallet, const std::string& base_path,
    const Amount& amount, uint32_t offset, uint32_t count,
    std::vector<Privkey>* privkey_list) {
  std::vector<UtxoData> list;
  for (uint32_t index=offset; index<(offset + count); ++index) {
    std::string path = base_path + "/" + std::to_string(index);
    auto key = wallet.GeneratePrivkeyData(net_type, path);
    UtxoData utxo;
    utxo.descriptor = "wpkh(" + key.ToString() + ")";
    utxo.amount = amount;
    Descriptor desc = Descriptor::Parse(utxo.descriptor);
    auto ref = desc.GetReference();
    utxo.locking_script = ref.GetLockingScript();
    utxo.address = ref.GenerateAddress(net_type);
    utxo.address_type  = AddressType::kP2wpkhAddress;
    auto txid_bytes = HashUtil::Sha256(wallet.GetSeed().GetHex() + "/" + path);
    utxo.txid = Txid(ByteData256(txid_bytes));
    utxo.vout = index;
    list.emplace_back(utxo);
    if (privkey_list != nullptr) {
      privkey_list->push_back(key.GetPrivkey());
    }
  }
  return list;
}


TEST(Psbt, UsecaseTest2) {
  static const std::string seed1 = "8bc106907003ea0b55f3ed4ce2fcf9a198d8c43f07e6ade8aacc5c20c33db12e";
  static const std::string seed2 = "d3e3539eafb6af1f0ae374ecffd33bed394f5eb2e39f8957be63c258ac32ca97";

  NetType net_type = NetType::kRegtest;
  HDWallet wallet1 = HDWallet(ByteData(g_psbt_seed1));
  HDWallet wallet2 = HDWallet(ByteData(g_psbt_seed2));
  std::string path1 = "44h/0h/0h/1/100";
  auto change_key1 = wallet1.GeneratePubkeyData(net_type, path1);
  std::string change_descriptor1 = "wpkh(" + change_key1.ToString() + ")";
  Descriptor change_desc1 = Descriptor::Parse(change_descriptor1);
  EXPECT_STREQ("wpkh([2a704760/44'/0'/0'/1/100]02f7f0d7d00289b7c5a581bc35276040c348de48fc414067f31ea26ad95c00550c)", change_descriptor1.c_str());

  std::vector<Privkey> privkey_list1;
  auto utxo_list1 = CfdTestGenerateUtxo(
    net_type, wallet1, "44h/0h/0h/1", Amount(10000000), 1, 11, &privkey_list1);
  std::vector<Privkey> privkey_list2;
  auto utxo_list2 = CfdTestGenerateUtxo(
    net_type, wallet2, "44h/0h/0h/1", Amount(50000000), 1, 3, &privkey_list2);

  Psbt psbt;
  std::string out_path1 = "44h/0h/0h/0/2";
  std::string out_path2 = "44h/0h/0h/0/2";
  auto out_key1 = wallet1.GeneratePubkeyData(net_type, out_path1);
  auto out_key2 = wallet2.GeneratePubkeyData(net_type, out_path2);
  auto out_addr1 = Address(NetType::kTestnet, WitnessVersion::kVersion0, out_key1.GetPubkey());
  auto out_addr2 = Address(NetType::kTestnet, WitnessVersion::kVersion0, out_key2.GetPubkey());

  // Creator
  // add txout (0.5btc * 2)
  Amount amount(50000000);
  try {
    psbt.AddTxOutData(amount, out_addr1, out_key1);
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
  }
  try {
    psbt.AddTxOutData(amount, out_addr2, out_key2);
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
  }

  // Updater
  // add txin
  Psbt psbt1;
  Psbt psbt2;
  uint32_t use_utxo1_count = 5;
  try {
    psbt1 = psbt;
    for (uint32_t index=0; index<use_utxo1_count; ++index) {
      psbt1.AddTxInData(utxo_list1[index]);
    }
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  try {
    psbt2 = psbt;
    psbt2.AddTxInData(utxo_list2[0]);
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  try {
    psbt.Join(psbt1);
    EXPECT_EQ(5, psbt.GetTxInCount());
    EXPECT_EQ(10000000, psbt.GetUtxoData(0).amount.GetSatoshiValue());
    EXPECT_EQ(10000000, psbt.GetUtxoData(4).amount.GetSatoshiValue());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }
  try {
    psbt.Join(psbt2);
    EXPECT_EQ(50000000, psbt.GetUtxoData(5).amount.GetSatoshiValue());
    EXPECT_EQ(utxo_list2[0].amount.GetSatoshiValue(), psbt.GetUtxoData(5).amount.GetSatoshiValue());
    EXPECT_EQ(utxo_list2[0].txid.GetHex(), psbt.GetUtxoData(5).txid.GetHex());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  // 70736274ff0100fd3e010200000006f834da7cb5e183fc815f7846227284660fa4a0a583be2bfb4a535bf425e531de0100000000ffffffff5aef6a3bce7624951ae5b9e8c0908e8ff74721eedf7a0994d5bf1efc7dee82810200000000ffffffffa92dd64952789efd1444938e9ce5b106e7280089c4dc4a78b3da7c7a2b69d93f0300000000fffffffffcf72c7bb8881945955e48f54e6081b51901836484e5c48a9925061029c405d10400000000ffffffffa0be1a4d9a22c832d2d20a3a084f010d09a9ab41fb3e2dedbd46eb0104578fb90500000000ffffffff71a141bf5b653e2c43773305d69a8c9876944470ca1e29e4b03a0953c4beb6950100000000ffffffff0280f0fa0200000000160014b322bddce633b851ac7370ab454f0b367a0654e580f0fa0200000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a555000000000001011f8096980000000000160014962c4e08f336d3afbc3415c9d359ae1040470520220602565248460b3c186decf13db06f0724fd50b47cc3e489c30076c8363d5038cefe182a7047602c000080000000800000008001000000010000000001011f80969800000000001600148bf09d60b7e34f34827d8dbcb1c76390a916ca822206022744cfb2436e156040ec1c8fe842d9ef9a18cb40ad41f312725390c35f7bd36b182a7047602c000080000000800000008001000000020000000001011f8096980000000000160014f1d3ee67829225eb892ccab01e22f6a777e7e21e220602e7e8dc236fa024369408d2ce4d8508048261abc297b811604da087ad71d13855182a7047602c000080000000800000008001000000030000000001011f809698000000000016001419d65f8328b2206d9970785660ec0d34808fb0752206033d874bf19b697cf6c639659547a583b86730164a5b6db01bf20a14eb9b6adb44182a7047602c000080000000800000008001000000040000000001011f809698000000000016001412b7954a75efc2a20e86e32dc2d78647d6700779220602fb061730dbde3c806b4a17a99f454c81282aecee6d46f4a975a94cdfd3a06504182a7047602c000080000000800000008001000000050000000001011f80f0fa0200000000160014978a90460e44671a52f49a09bb59cc6794b63c89220603e3d244a3967e0b87765fda86c5ff38885f74993953b9584388aef30b26af6aec189d6b6d862c0000800000008000000080010000000100000000220203473bfc8c770c1b220a2e7aae4badf6c0d7eaf29028d5b29d3438012bb289ef81182a7047602c00008000000080000000800000000002000000002202036474aff2633c351865539fb52b62b9d6fb9e4e23576628e1f0a0a7993458e06c189d6b6d862c0000800000008000000080000000000200000000
  EXPECT_STREQ("cHNidP8BAP0+AQIAAAAG+DTafLXhg/yBX3hGInKEZg+koKWDviv7SlNb9CXlMd4BAAAAAP////9a72o7znYklRrluejAkI6P90ch7t96CZTVvx78fe6CgQIAAAAA/////6kt1klSeJ79FESTjpzlsQbnKACJxNxKeLPafHoradk/AwAAAAD//////Pcse7iIGUWVXkj1TmCBtRkBg2SE5cSKmSUGECnEBdEEAAAAAP////+gvhpNmiLIMtLSCjoITwENCamrQfs+Le29RusBBFePuQUAAAAA/////3GhQb9bZT4sQ3czBdaajJh2lERwyh4p5LA6CVPEvraVAQAAAAD/////AoDw+gIAAAAAFgAUsyK93OYzuFGsc3CrRU8LNnoGVOWA8PoCAAAAABYAFMq4xTpuj8ApbRzTkVowfVHEkaVVAAAAAAABAR+AlpgAAAAAABYAFJYsTgjzNtOvvDQVydNZrhBARwUgIgYCVlJIRgs8GG3s8T2wbwck/VC0fMPkicMAdsg2PVA4zv4YKnBHYCwAAIAAAACAAAAAgAEAAAABAAAAAAEBH4CWmAAAAAAAFgAUi/CdYLfjTzSCfY28scdjkKkWyoIiBgInRM+yQ24VYEDsHI/oQtnvmhjLQK1B8xJyU5DDX3vTaxgqcEdgLAAAgAAAAIAAAACAAQAAAAIAAAAAAQEfgJaYAAAAAAAWABTx0+5ngpIl64ksyrAeIvand+fiHiIGAufo3CNvoCQ2lAjSzk2FCASCYavCl7gRYE2gh61x0ThVGCpwR2AsAACAAAAAgAAAAIABAAAAAwAAAAABAR+AlpgAAAAAABYAFBnWX4MosiBtmXB4VmDsDTSAj7B1IgYDPYdL8ZtpfPbGOWWVR6WDuGcwFkpbbbAb8goU65tq20QYKnBHYCwAAIAAAACAAAAAgAEAAAAEAAAAAAEBH4CWmAAAAAAAFgAUEreVSnXvwqIOhuMtwteGR9ZwB3kiBgL7Bhcw2948gGtKF6mfRUyBKCrs7m1G9Kl1qUzf06BlBBgqcEdgLAAAgAAAAIAAAACAAQAAAAUAAAAAAQEfgPD6AgAAAAAWABSXipBGDkRnGlL0mgm7WcxnlLY8iSIGA+PSRKOWfguHdl/ahsX/OIhfdJk5U7lYQ4iu8wsmr2rsGJ1rbYYsAACAAAAAgAAAAIABAAAAAQAAAAAiAgNHO/yMdwwbIgoueq5LrfbA1+rykCjVsp00OAErsonvgRgqcEdgLAAAgAAAAIAAAACAAAAAAAIAAAAAIgIDZHSv8mM8NRhlU5+1K2K51vueTiNXZijh8KCnmTRY4GwYnWtthiwAAIAAAACAAAAAgAAAAAACAAAAAA==", psbt.GetBase64().c_str());

  // Updater (for fee)
  std::vector<UtxoData> selected_utxos;
  std::vector<Privkey> target_privkey_list1;
  try {
    Amount fee;
    std::vector<UtxoData> selection_utxo;
    std::copy(utxo_list1.begin() + use_utxo1_count + 1, utxo_list1.end(),
        std::back_inserter(selection_utxo));
    CoinSelectionOption option;
    option.SetEffectiveFeeBaserate(2.0);
    option.SetKnapsackMinimumChange(0);
    EXPECT_EQ(5, selection_utxo.size());
    EXPECT_EQ(10000000, selection_utxo[0].amount.GetSatoshiValue());
    EXPECT_EQ(6, psbt.GetTxInCount());

    EXPECT_STREQ("8b3a1f738009f99fd8b01e1ad95c5468e8fb25d5373df19697d78eec957c423c", selection_utxo[0].txid.GetHex().c_str());
    EXPECT_EQ(7, selection_utxo[0].vout);
    EXPECT_STREQ("wpkh([2a704760/44'/0'/0'/1/7]03c02325c328fed622a9d88f8f5318e05261a3c3a967b7211d7d67657e2b5e9fbe)", selection_utxo[0].descriptor.c_str());
    EXPECT_STREQ("0e5b0e16148da878ee8d9f0524ac0962a22dbbb66e2dc8a6237a1d4c8c4ea9cb", selection_utxo[1].txid.GetHex().c_str());
    EXPECT_EQ(8, selection_utxo[1].vout);
    EXPECT_STREQ("wpkh([2a704760/44'/0'/0'/1/8]02f505e0b5885959bd2f7e68dd73915940a5540e05c41a42f6c598ceb21bdc55f3)", selection_utxo[1].descriptor.c_str());
    EXPECT_STREQ("016b36a0a9df54d55f7b907aba8fdd3d90d797eee2bc46c5ed7dced41218e674", selection_utxo[2].txid.GetHex().c_str());
    EXPECT_EQ(9, selection_utxo[2].vout);
    EXPECT_STREQ("wpkh([2a704760/44'/0'/0'/1/9]02f7af7e1c3c8627e0e662ba04ec003879d9aeaeba5a02a1e24804ef4d7b497db4)", selection_utxo[2].descriptor.c_str());
    EXPECT_STREQ("dcc65ebfb8535f96f8a51e07d5c0a94f965000bd5765159a3cc27e4edd658c69", selection_utxo[3].txid.GetHex().c_str());
    EXPECT_EQ(10, selection_utxo[3].vout);
    EXPECT_STREQ("wpkh([2a704760/44'/0'/0'/1/10]03c04e76f9bc7d6433a6eb7c0c3c446213c5f361dae22929483f23718e1cee4ece)", selection_utxo[3].descriptor.c_str());
    EXPECT_STREQ("0230e7471501f9432f727583f2bc17c11f5c1132140319ba2d7a5a0145684c73", selection_utxo[4].txid.GetHex().c_str());
    EXPECT_EQ(11, selection_utxo[4].vout);
    EXPECT_STREQ("wpkh([2a704760/44'/0'/0'/1/11]03798ff13e5d17c6f8c1d69ff18d516613be2fa52149b3a8c60ef959d1075a5889)", selection_utxo[4].descriptor.c_str());

    selected_utxos = psbt.FundTransaction(selection_utxo, 2.0,
        &change_desc1, &fee, &option);
    EXPECT_EQ(fee.GetSatoshiValue(), 1166);
    
    for (uint32_t index=0; index<use_utxo1_count; ++index) {
      target_privkey_list1.push_back(privkey_list1[index]);
    }
    for (const auto& select_utxo : selected_utxos) {
      for (size_t index=use_utxo1_count; index<utxo_list1.size(); ++index) {
        if ((select_utxo.vout == utxo_list1[index].vout) &&
            select_utxo.txid.Equals(utxo_list1[index].txid)) {
          target_privkey_list1.push_back(privkey_list1[index]);
        }
      }
    }
    EXPECT_STREQ("cHNidP8BAP2GAQIAAAAH+DTafLXhg/yBX3hGInKEZg+koKWDviv7SlNb9CXlMd4BAAAAAP////9a72o7znYklRrluejAkI6P90ch7t96CZTVvx78fe6CgQIAAAAA/////6kt1klSeJ79FESTjpzlsQbnKACJxNxKeLPafHoradk/AwAAAAD//////Pcse7iIGUWVXkj1TmCBtRkBg2SE5cSKmSUGECnEBdEEAAAAAP////+gvhpNmiLIMtLSCjoITwENCamrQfs+Le29RusBBFePuQUAAAAA/////3GhQb9bZT4sQ3czBdaajJh2lERwyh4p5LA6CVPEvraVAQAAAAD/////PEJ8leyO15eW8T031SX76GhUXNkaHrDYn/kJgHMfOosHAAAAAP////8DgPD6AgAAAAAWABSzIr3c5jO4UaxzcKtFTws2egZU5YDw+gIAAAAAFgAUyrjFOm6PwCltHNORWjB9UcSRpVXykZgAAAAAABYAFCtuFro44oBQD4CyxXBu8C841ZCBAAAAAAABAR+AlpgAAAAAABYAFJYsTgjzNtOvvDQVydNZrhBARwUgIgYCVlJIRgs8GG3s8T2wbwck/VC0fMPkicMAdsg2PVA4zv4YKnBHYCwAAIAAAACAAAAAgAEAAAABAAAAAAEBH4CWmAAAAAAAFgAUi/CdYLfjTzSCfY28scdjkKkWyoIiBgInRM+yQ24VYEDsHI/oQtnvmhjLQK1B8xJyU5DDX3vTaxgqcEdgLAAAgAAAAIAAAACAAQAAAAIAAAAAAQEfgJaYAAAAAAAWABTx0+5ngpIl64ksyrAeIvand+fiHiIGAufo3CNvoCQ2lAjSzk2FCASCYavCl7gRYE2gh61x0ThVGCpwR2AsAACAAAAAgAAAAIABAAAAAwAAAAABAR+AlpgAAAAAABYAFBnWX4MosiBtmXB4VmDsDTSAj7B1IgYDPYdL8ZtpfPbGOWWVR6WDuGcwFkpbbbAb8goU65tq20QYKnBHYCwAAIAAAACAAAAAgAEAAAAEAAAAAAEBH4CWmAAAAAAAFgAUEreVSnXvwqIOhuMtwteGR9ZwB3kiBgL7Bhcw2948gGtKF6mfRUyBKCrs7m1G9Kl1qUzf06BlBBgqcEdgLAAAgAAAAIAAAACAAQAAAAUAAAAAAQEfgPD6AgAAAAAWABSXipBGDkRnGlL0mgm7WcxnlLY8iSIGA+PSRKOWfguHdl/ahsX/OIhfdJk5U7lYQ4iu8wsmr2rsGJ1rbYYsAACAAAAAgAAAAIABAAAAAQAAAAABAR+AlpgAAAAAABYAFBzoeOOg2js0MIeX/ey3YiH4VBivIgYDwCMlwyj+1iKp2I+PUxjgUmGjw6lntyEdfWdlfiten74YKnBHYCwAAIAAAACAAAAAgAEAAAAHAAAAACICA0c7/Ix3DBsiCi56rkut9sDX6vKQKNWynTQ4ASuyie+BGCpwR2AsAACAAAAAgAAAAIAAAAAAAgAAAAAiAgNkdK/yYzw1GGVTn7UrYrnW+55OI1dmKOHwoKeZNFjgbBida22GLAAAgAAAAIAAAACAAAAAAAIAAAAAIgIC9/DX0AKJt8Wlgbw1J2BAw0jeSPxBQGfzHqJq2VwAVQwYKnBHYCwAAIAAAACAAAAAgAEAAABkAAAAAA==", psbt.GetBase64().c_str());

  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  // Signer
  try {
    psbt1 = psbt;
    for (const auto& privkey : privkey_list1) {
      psbt1.Sign(privkey);
    }
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }
  try {
    psbt2 = psbt;
    psbt2.Sign(privkey_list2[0]);
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  // Combiner
  try {
    psbt.Combine(psbt1);
    psbt.Combine(psbt2);
    EXPECT_STREQ("70736274ff0100fd86010200000007f834da7cb5e183fc815f7846227284660fa4a0a583be2bfb4a535bf425e531de0100000000ffffffff5aef6a3bce7624951ae5b9e8c0908e8ff74721eedf7a0994d5bf1efc7dee82810200000000ffffffffa92dd64952789efd1444938e9ce5b106e7280089c4dc4a78b3da7c7a2b69d93f0300000000fffffffffcf72c7bb8881945955e48f54e6081b51901836484e5c48a9925061029c405d10400000000ffffffffa0be1a4d9a22c832d2d20a3a084f010d09a9ab41fb3e2dedbd46eb0104578fb90500000000ffffffff71a141bf5b653e2c43773305d69a8c9876944470ca1e29e4b03a0953c4beb6950100000000ffffffff3c427c95ec8ed79796f13d37d525fbe868545cd91a1eb0d89ff90980731f3a8b0700000000ffffffff0380f0fa0200000000160014b322bddce633b851ac7370ab454f0b367a0654e580f0fa0200000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a555f2919800000000001600142b6e16ba38e280500f80b2c5706ef02f38d59081000000000001011f8096980000000000160014962c4e08f336d3afbc3415c9d359ae1040470520220202565248460b3c186decf13db06f0724fd50b47cc3e489c30076c8363d5038cefe473044022016d6be246613d8f20a98f79651ba6402352478bde005ba292adee40c3060f97a022061e5e264be9a892f4579bcbcd5c660405d3b95adb807c8b54caf3dc22f75f9fd01220602565248460b3c186decf13db06f0724fd50b47cc3e489c30076c8363d5038cefe182a7047602c000080000000800000008001000000010000000001011f80969800000000001600148bf09d60b7e34f34827d8dbcb1c76390a916ca822202022744cfb2436e156040ec1c8fe842d9ef9a18cb40ad41f312725390c35f7bd36b4730440220622277094f292704dfbcd95c951b95f83f8ae85b0584d0a6058548a7a0ae814e02200b913ac3fcce9780911e927997442af8de0945f03a9576a2c300bdebe08c6182012206022744cfb2436e156040ec1c8fe842d9ef9a18cb40ad41f312725390c35f7bd36b182a7047602c000080000000800000008001000000020000000001011f8096980000000000160014f1d3ee67829225eb892ccab01e22f6a777e7e21e220202e7e8dc236fa024369408d2ce4d8508048261abc297b811604da087ad71d1385547304402202dc3146d944ac124e6964730524e73b3600ce84ec4c286cbcccfe7eb25cfb2a30220701661116a179ce213c14280073253c3467ef8e8a7324618ac263e337046b3a601220602e7e8dc236fa024369408d2ce4d8508048261abc297b811604da087ad71d13855182a7047602c000080000000800000008001000000030000000001011f809698000000000016001419d65f8328b2206d9970785660ec0d34808fb0752202033d874bf19b697cf6c639659547a583b86730164a5b6db01bf20a14eb9b6adb44473044022062b755ad1f11cd001bb9744788c4db87d90253d337f37048cb29529eacd854050220483bbc41077c7a31cbaf7dc02acad05e28e42d2a3224c99cfb31064fa21015b0012206033d874bf19b697cf6c639659547a583b86730164a5b6db01bf20a14eb9b6adb44182a7047602c000080000000800000008001000000040000000001011f809698000000000016001412b7954a75efc2a20e86e32dc2d78647d6700779220202fb061730dbde3c806b4a17a99f454c81282aecee6d46f4a975a94cdfd3a06504473044022065ef50669cb7804a837b11c727f8a4891de3b408544c5b817fed41b3a2cdbd67022046b380b373c2368ffdce73a9b2b0f634ef83915aea9e677133caeae861a24f8801220602fb061730dbde3c806b4a17a99f454c81282aecee6d46f4a975a94cdfd3a06504182a7047602c000080000000800000008001000000050000000001011f80f0fa0200000000160014978a90460e44671a52f49a09bb59cc6794b63c89220203e3d244a3967e0b87765fda86c5ff38885f74993953b9584388aef30b26af6aec4730440220779520daeb0ae58727df2578543ce32a060ed74eca86c44820e52acec43013d10220097b0815506262bf9728276db3aa227834959187455dc3f191f9fe80aa30955201220603e3d244a3967e0b87765fda86c5ff38885f74993953b9584388aef30b26af6aec189d6b6d862c000080000000800000008001000000010000000001011f80969800000000001600141ce878e3a0da3b34308797fdecb76221f85418af220203c02325c328fed622a9d88f8f5318e05261a3c3a967b7211d7d67657e2b5e9fbe47304402200fd024910c6207d2679ad1b9bb0ed8c0ff8d55aa603ef7c499917da20a3f417a0220086dfa0cce871385e24da90b7e20f5f1e1f9f7d707c5562bcfa66a855d7d220801220603c02325c328fed622a9d88f8f5318e05261a3c3a967b7211d7d67657e2b5e9fbe182a7047602c0000800000008000000080010000000700000000220203473bfc8c770c1b220a2e7aae4badf6c0d7eaf29028d5b29d3438012bb289ef81182a7047602c00008000000080000000800000000002000000002202036474aff2633c351865539fb52b62b9d6fb9e4e23576628e1f0a0a7993458e06c189d6b6d862c0000800000008000000080000000000200000000220202f7f0d7d00289b7c5a581bc35276040c348de48fc414067f31ea26ad95c00550c182a7047602c0000800000008000000080010000006400000000", psbt.GetData().GetHex().c_str());
    EXPECT_STREQ("cHNidP8BAP2GAQIAAAAH+DTafLXhg/yBX3hGInKEZg+koKWDviv7SlNb9CXlMd4BAAAAAP////9a72o7znYklRrluejAkI6P90ch7t96CZTVvx78fe6CgQIAAAAA/////6kt1klSeJ79FESTjpzlsQbnKACJxNxKeLPafHoradk/AwAAAAD//////Pcse7iIGUWVXkj1TmCBtRkBg2SE5cSKmSUGECnEBdEEAAAAAP////+gvhpNmiLIMtLSCjoITwENCamrQfs+Le29RusBBFePuQUAAAAA/////3GhQb9bZT4sQ3czBdaajJh2lERwyh4p5LA6CVPEvraVAQAAAAD/////PEJ8leyO15eW8T031SX76GhUXNkaHrDYn/kJgHMfOosHAAAAAP////8DgPD6AgAAAAAWABSzIr3c5jO4UaxzcKtFTws2egZU5YDw+gIAAAAAFgAUyrjFOm6PwCltHNORWjB9UcSRpVXykZgAAAAAABYAFCtuFro44oBQD4CyxXBu8C841ZCBAAAAAAABAR+AlpgAAAAAABYAFJYsTgjzNtOvvDQVydNZrhBARwUgIgICVlJIRgs8GG3s8T2wbwck/VC0fMPkicMAdsg2PVA4zv5HMEQCIBbWviRmE9jyCpj3llG6ZAI1JHi94AW6KSre5AwwYPl6AiBh5eJkvpqJL0V5vLzVxmBAXTuVrbgHyLVMrz3CL3X5/QEiBgJWUkhGCzwYbezxPbBvByT9ULR8w+SJwwB2yDY9UDjO/hgqcEdgLAAAgAAAAIAAAACAAQAAAAEAAAAAAQEfgJaYAAAAAAAWABSL8J1gt+NPNIJ9jbyxx2OQqRbKgiICAidEz7JDbhVgQOwcj+hC2e+aGMtArUHzEnJTkMNfe9NrRzBEAiBiIncJTyknBN+82VyVG5X4P4roWwWE0KYFhUinoK6BTgIgC5E6w/zOl4CRHpJ5l0Qq+N4JRfA6lXaiwwC96+CMYYIBIgYCJ0TPskNuFWBA7ByP6ELZ75oYy0CtQfMSclOQw19702sYKnBHYCwAAIAAAACAAAAAgAEAAAACAAAAAAEBH4CWmAAAAAAAFgAU8dPuZ4KSJeuJLMqwHiL2p3fn4h4iAgLn6Nwjb6AkNpQI0s5NhQgEgmGrwpe4EWBNoIetcdE4VUcwRAIgLcMUbZRKwSTmlkcwUk5zs2AM6E7EwobLzM/n6yXPsqMCIHAWYRFqF5ziE8FCgAcyU8NGfvjopzJGGKwmPjNwRrOmASIGAufo3CNvoCQ2lAjSzk2FCASCYavCl7gRYE2gh61x0ThVGCpwR2AsAACAAAAAgAAAAIABAAAAAwAAAAABAR+AlpgAAAAAABYAFBnWX4MosiBtmXB4VmDsDTSAj7B1IgIDPYdL8ZtpfPbGOWWVR6WDuGcwFkpbbbAb8goU65tq20RHMEQCIGK3Va0fEc0AG7l0R4jE24fZAlPTN/NwSMspUp6s2FQFAiBIO7xBB3x6McuvfcAqytBeKOQtKjIkyZz7MQZPohAVsAEiBgM9h0vxm2l89sY5ZZVHpYO4ZzAWSlttsBvyChTrm2rbRBgqcEdgLAAAgAAAAIAAAACAAQAAAAQAAAAAAQEfgJaYAAAAAAAWABQSt5VKde/Cog6G4y3C14ZH1nAHeSICAvsGFzDb3jyAa0oXqZ9FTIEoKuzubUb0qXWpTN/ToGUERzBEAiBl71BmnLeASoN7Eccn+KSJHeO0CFRMW4F/7UGzos29ZwIgRrOAs3PCNo/9znOpsrD2NO+DkVrqnmdxM8rq6GGiT4gBIgYC+wYXMNvePIBrShepn0VMgSgq7O5tRvSpdalM39OgZQQYKnBHYCwAAIAAAACAAAAAgAEAAAAFAAAAAAEBH4Dw+gIAAAAAFgAUl4qQRg5EZxpS9JoJu1nMZ5S2PIkiAgPj0kSjln4Lh3Zf2obF/ziIX3SZOVO5WEOIrvMLJq9q7EcwRAIgd5Ug2usK5Ycn3yV4VDzjKgYO107KhsRIIOUqzsQwE9ECIAl7CBVQYmK/lygnbbOqIng0lZGHRV3D8ZH5/oCqMJVSASIGA+PSRKOWfguHdl/ahsX/OIhfdJk5U7lYQ4iu8wsmr2rsGJ1rbYYsAACAAAAAgAAAAIABAAAAAQAAAAABAR+AlpgAAAAAABYAFBzoeOOg2js0MIeX/ey3YiH4VBivIgIDwCMlwyj+1iKp2I+PUxjgUmGjw6lntyEdfWdlfiten75HMEQCIA/QJJEMYgfSZ5rRubsO2MD/jVWqYD73xJmRfaIKP0F6AiAIbfoMzocTheJNqQt+IPXx4fn31wfFVivPpmqFXX0iCAEiBgPAIyXDKP7WIqnYj49TGOBSYaPDqWe3IR19Z2V+K16fvhgqcEdgLAAAgAAAAIAAAACAAQAAAAcAAAAAIgIDRzv8jHcMGyIKLnquS632wNfq8pAo1bKdNDgBK7KJ74EYKnBHYCwAAIAAAACAAAAAgAAAAAACAAAAACICA2R0r/JjPDUYZVOftStiudb7nk4jV2Yo4fCgp5k0WOBsGJ1rbYYsAACAAAAAgAAAAIAAAAAAAgAAAAAiAgL38NfQAom3xaWBvDUnYEDDSN5I/EFAZ/MeomrZXABVDBgqcEdgLAAAgAAAAIAAAACAAQAAAGQAAAAA", psbt.GetBase64().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  // Input Finalizer
  try {
    EXPECT_FALSE(psbt.IsFinalized());
    OutPoint outpoint1(utxo_list1[0].txid, utxo_list1[0].vout);
    EXPECT_FALSE(psbt.IsFinalizedInput(outpoint1));
    psbt.Finalize();
    EXPECT_TRUE(psbt.IsFinalizedInput(outpoint1));
    EXPECT_STREQ("70736274ff0100fd86010200000007f834da7cb5e183fc815f7846227284660fa4a0a583be2bfb4a535bf425e531de0100000000ffffffff5aef6a3bce7624951ae5b9e8c0908e8ff74721eedf7a0994d5bf1efc7dee82810200000000ffffffffa92dd64952789efd1444938e9ce5b106e7280089c4dc4a78b3da7c7a2b69d93f0300000000fffffffffcf72c7bb8881945955e48f54e6081b51901836484e5c48a9925061029c405d10400000000ffffffffa0be1a4d9a22c832d2d20a3a084f010d09a9ab41fb3e2dedbd46eb0104578fb90500000000ffffffff71a141bf5b653e2c43773305d69a8c9876944470ca1e29e4b03a0953c4beb6950100000000ffffffff3c427c95ec8ed79796f13d37d525fbe868545cd91a1eb0d89ff90980731f3a8b0700000000ffffffff0380f0fa0200000000160014b322bddce633b851ac7370ab454f0b367a0654e580f0fa0200000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a555f2919800000000001600142b6e16ba38e280500f80b2c5706ef02f38d59081000000000001011f8096980000000000160014962c4e08f336d3afbc3415c9d359ae104047052001086b02473044022016d6be246613d8f20a98f79651ba6402352478bde005ba292adee40c3060f97a022061e5e264be9a892f4579bcbcd5c660405d3b95adb807c8b54caf3dc22f75f9fd012102565248460b3c186decf13db06f0724fd50b47cc3e489c30076c8363d5038cefe0001011f80969800000000001600148bf09d60b7e34f34827d8dbcb1c76390a916ca8201086b024730440220622277094f292704dfbcd95c951b95f83f8ae85b0584d0a6058548a7a0ae814e02200b913ac3fcce9780911e927997442af8de0945f03a9576a2c300bdebe08c61820121022744cfb2436e156040ec1c8fe842d9ef9a18cb40ad41f312725390c35f7bd36b0001011f8096980000000000160014f1d3ee67829225eb892ccab01e22f6a777e7e21e01086b0247304402202dc3146d944ac124e6964730524e73b3600ce84ec4c286cbcccfe7eb25cfb2a30220701661116a179ce213c14280073253c3467ef8e8a7324618ac263e337046b3a6012102e7e8dc236fa024369408d2ce4d8508048261abc297b811604da087ad71d138550001011f809698000000000016001419d65f8328b2206d9970785660ec0d34808fb07501086b02473044022062b755ad1f11cd001bb9744788c4db87d90253d337f37048cb29529eacd854050220483bbc41077c7a31cbaf7dc02acad05e28e42d2a3224c99cfb31064fa21015b00121033d874bf19b697cf6c639659547a583b86730164a5b6db01bf20a14eb9b6adb440001011f809698000000000016001412b7954a75efc2a20e86e32dc2d78647d670077901086b02473044022065ef50669cb7804a837b11c727f8a4891de3b408544c5b817fed41b3a2cdbd67022046b380b373c2368ffdce73a9b2b0f634ef83915aea9e677133caeae861a24f88012102fb061730dbde3c806b4a17a99f454c81282aecee6d46f4a975a94cdfd3a065040001011f80f0fa0200000000160014978a90460e44671a52f49a09bb59cc6794b63c8901086b024730440220779520daeb0ae58727df2578543ce32a060ed74eca86c44820e52acec43013d10220097b0815506262bf9728276db3aa227834959187455dc3f191f9fe80aa309552012103e3d244a3967e0b87765fda86c5ff38885f74993953b9584388aef30b26af6aec0001011f80969800000000001600141ce878e3a0da3b34308797fdecb76221f85418af01086b0247304402200fd024910c6207d2679ad1b9bb0ed8c0ff8d55aa603ef7c499917da20a3f417a0220086dfa0cce871385e24da90b7e20f5f1e1f9f7d707c5562bcfa66a855d7d2208012103c02325c328fed622a9d88f8f5318e05261a3c3a967b7211d7d67657e2b5e9fbe00220203473bfc8c770c1b220a2e7aae4badf6c0d7eaf29028d5b29d3438012bb289ef81182a7047602c00008000000080000000800000000002000000002202036474aff2633c351865539fb52b62b9d6fb9e4e23576628e1f0a0a7993458e06c189d6b6d862c0000800000008000000080000000000200000000220202f7f0d7d00289b7c5a581bc35276040c348de48fc414067f31ea26ad95c00550c182a7047602c0000800000008000000080010000006400000000", psbt.GetData().GetHex().c_str());

    psbt.Verify();
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  // Transaction Extractor
  try {
    EXPECT_TRUE(psbt.IsFinalized());
    EXPECT_EQ(psbt.GetFeeAmount().GetSatoshiValue(), 1166);
    auto tx = psbt.ExtractTransaction();
    EXPECT_STREQ("02000000000107f834da7cb5e183fc815f7846227284660fa4a0a583be2bfb4a535bf425e531de0100000000ffffffff5aef6a3bce7624951ae5b9e8c0908e8ff74721eedf7a0994d5bf1efc7dee82810200000000ffffffffa92dd64952789efd1444938e9ce5b106e7280089c4dc4a78b3da7c7a2b69d93f0300000000fffffffffcf72c7bb8881945955e48f54e6081b51901836484e5c48a9925061029c405d10400000000ffffffffa0be1a4d9a22c832d2d20a3a084f010d09a9ab41fb3e2dedbd46eb0104578fb90500000000ffffffff71a141bf5b653e2c43773305d69a8c9876944470ca1e29e4b03a0953c4beb6950100000000ffffffff3c427c95ec8ed79796f13d37d525fbe868545cd91a1eb0d89ff90980731f3a8b0700000000ffffffff0380f0fa0200000000160014b322bddce633b851ac7370ab454f0b367a0654e580f0fa0200000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a555f2919800000000001600142b6e16ba38e280500f80b2c5706ef02f38d5908102473044022016d6be246613d8f20a98f79651ba6402352478bde005ba292adee40c3060f97a022061e5e264be9a892f4579bcbcd5c660405d3b95adb807c8b54caf3dc22f75f9fd012102565248460b3c186decf13db06f0724fd50b47cc3e489c30076c8363d5038cefe024730440220622277094f292704dfbcd95c951b95f83f8ae85b0584d0a6058548a7a0ae814e02200b913ac3fcce9780911e927997442af8de0945f03a9576a2c300bdebe08c61820121022744cfb2436e156040ec1c8fe842d9ef9a18cb40ad41f312725390c35f7bd36b0247304402202dc3146d944ac124e6964730524e73b3600ce84ec4c286cbcccfe7eb25cfb2a30220701661116a179ce213c14280073253c3467ef8e8a7324618ac263e337046b3a6012102e7e8dc236fa024369408d2ce4d8508048261abc297b811604da087ad71d1385502473044022062b755ad1f11cd001bb9744788c4db87d90253d337f37048cb29529eacd854050220483bbc41077c7a31cbaf7dc02acad05e28e42d2a3224c99cfb31064fa21015b00121033d874bf19b697cf6c639659547a583b86730164a5b6db01bf20a14eb9b6adb4402473044022065ef50669cb7804a837b11c727f8a4891de3b408544c5b817fed41b3a2cdbd67022046b380b373c2368ffdce73a9b2b0f634ef83915aea9e677133caeae861a24f88012102fb061730dbde3c806b4a17a99f454c81282aecee6d46f4a975a94cdfd3a06504024730440220779520daeb0ae58727df2578543ce32a060ed74eca86c44820e52acec43013d10220097b0815506262bf9728276db3aa227834959187455dc3f191f9fe80aa309552012103e3d244a3967e0b87765fda86c5ff38885f74993953b9584388aef30b26af6aec0247304402200fd024910c6207d2679ad1b9bb0ed8c0ff8d55aa603ef7c499917da20a3f417a0220086dfa0cce871385e24da90b7e20f5f1e1f9f7d707c5562bcfa66a855d7d2208012103c02325c328fed622a9d88f8f5318e05261a3c3a967b7211d7d67657e2b5e9fbe00000000", tx.GetHex().c_str());

    TransactionContext context(tx.GetHex());
    auto utxos = psbt.GetUtxoDataAll();
    EXPECT_EQ(10000000, utxos[0].amount.GetSatoshiValue());
    EXPECT_EQ(10000000, utxos[4].amount.GetSatoshiValue());
    EXPECT_STREQ("b98f570401eb46bded2d3efb41aba9090d014f083a0ad2d232c8229a4d1abea0", utxos[4].txid.GetHex().c_str());
    UtxoData utxo0 = utxos[0];
    UtxoData utxo4 = utxos[4];
    UtxoData utxo5 = utxos[5];
    context.CollectInputUtxo(utxos);
    context.Verify();

    auto sighash = context.GetSignatureHash(0, ByteData("76a914962c4e08f336d3afbc3415c9d359ae104047052088ac"), SigHashType(),
        utxos[0].amount, WitnessVersion::kVersion0);
    EXPECT_STREQ(
      "b330738168b0df580797fa0e5eba6287b16f7b881df208c7045008f0d193e2c1",
      sighash.GetHex().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }
}

TEST(Psbt, UsecaseMultisig2) {
  Psbt psbt;
  
  HDWallet wallet1 = HDWallet(ByteData(g_psbt_seed1));
  HDWallet wallet2 = HDWallet(ByteData(g_psbt_seed2));
  std::string path1 = "44h/0h/0h/0/12";
  auto key1 = wallet1.GeneratePubkeyData(NetType::kTestnet, path1);
  auto key2 = wallet2.GeneratePubkeyData(NetType::kTestnet, path1);
  auto out_multisig = ScriptUtil::CreateMultisigRedeemScript(
    2, std::vector<Pubkey>{key1.GetPubkey(), key2.GetPubkey()});
  auto addr = Address(NetType::kTestnet, WitnessVersion::kVersion0, out_multisig);

  std::string in_path = "44h/0h/0h/0/11";
  auto key_in1 = wallet1.GeneratePrivkeyData(NetType::kTestnet, in_path);
  auto key_in2 = wallet2.GeneratePrivkeyData(NetType::kTestnet, in_path);
  auto multisig = ScriptUtil::CreateMultisigRedeemScript(
    2, std::vector<Pubkey>{key_in1.GetPubkey(), key_in2.GetPubkey()});
  EXPECT_STREQ("522103a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf32103f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b4752ae", multisig.GetHex().c_str());

  EXPECT_STREQ("[2a704760/44'/0'/0'/0/12]02906d399f6dbbecc898d4b8de3f497474c86a41f2cf36d71f95e5ca06b074867b", key1.ToString().c_str());
  EXPECT_STREQ("[9d6b6d86/44'/0'/0'/0/12]02622c7974ec32de75afa3733b09a6c33d0dea514b29faaccfb0991774f4622242", key2.ToString().c_str());
  EXPECT_STREQ("ac9aa506601b23ea7362dfcb9350ff3c2edeb67b334258d6489889e67dfa417f", key_in1.GetPrivkey().GetHex().c_str());
  EXPECT_STREQ("973e1d45dca188b0c0ac67bcc5de3ccdbe479d7505bdc43c57231c75f8db55e3", key_in2.GetPrivkey().GetHex().c_str());
  EXPECT_STREQ("[2a704760/44'/0'/0'/0/11]03a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf3", key_in1.ToString().c_str());
  EXPECT_STREQ("[9d6b6d86/44'/0'/0'/0/11]03f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b47", key_in2.ToString().c_str());

  Transaction utxo_tx(g_psbt_utxo_sh_wsh_multi);
  TxIn txin(Txid("544d545a1e53becbf9dd9b0e424e1189b8f6e46d6bc36b191816d341c2d32f69"), 0, 0xffffffff);

  try {
    psbt.AddTxIn(txin);
    psbt.SetTxInUtxo(0, utxo_tx.GetTxOut(0), multisig, KeyData());
    psbt.SetTxInSighashType(0, SigHashType());
    EXPECT_STREQ("522103a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf32103f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b4752ae",
        psbt.GetTxInRedeemScriptDirect(0, true, true).GetHex().c_str());
    EXPECT_STREQ("00209c4dacb25ebb8ada8bbb1addb869dea4d8170cc951f1d9694b2154e1583276c9",
        psbt.GetTxInRedeemScriptDirect(0, true, false).GetHex().c_str());
    EXPECT_STREQ("70736274ff0100330200000001692fd3c241d31618196bc36b6de4f6b889114e420e9bddf9cbbe531e5a544d540000000000ffffffff0000000000000101200858cd1d0000000017a914945fb50391a70637c1ffc5ab7fb65308c2f23175870103040100000001042200209c4dacb25ebb8ada8bbb1addb869dea4d8170cc951f1d9694b2154e1583276c9010547522103a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf32103f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b4752ae00", psbt.GetData().GetHex().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  Amount amount(499993360);
  try {
    psbt.AddTxOut(addr.GetLockingScript(), amount);
    psbt.SetTxOutData(0, out_multisig, std::vector<KeyData>{key1, key2});
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  Psbt psbt1;
  try {
    psbt1.AddTxIn(txin);
    psbt1.SetTxInUtxo(0, utxo_tx.GetTxOut(0), multisig, key_in1);
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  Psbt psbt2;
  try {
    psbt2.AddTxIn(txin);
    psbt2.SetTxInUtxo(0, utxo_tx.GetTxOut(0), multisig, key_in2);
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  try {
    psbt.Join(psbt1);
    psbt.Join(psbt2);
    EXPECT_STREQ("70736274ff01005e0200000001692fd3c241d31618196bc36b6de4f6b889114e420e9bddf9cbbe531e5a544d540000000000ffffffff01104bcd1d000000002200203cad0619de67e6247a76a102813635c053457c6ba4fde4ac1ffd8148d70e4bcc00000000000101200858cd1d0000000017a914945fb50391a70637c1ffc5ab7fb65308c2f23175870103040100000001042200209c4dacb25ebb8ada8bbb1addb869dea4d8170cc951f1d9694b2154e1583276c9010547522103a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf32103f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b4752ae220603a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf3182a7047602c0000800000008000000080000000000b000000220603f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b47189d6b6d862c0000800000008000000080000000000b00000000010147522102906d399f6dbbecc898d4b8de3f497474c86a41f2cf36d71f95e5ca06b074867b2102622c7974ec32de75afa3733b09a6c33d0dea514b29faaccfb0991774f462224252ae220202622c7974ec32de75afa3733b09a6c33d0dea514b29faaccfb0991774f4622242189d6b6d862c0000800000008000000080000000000c000000220202906d399f6dbbecc898d4b8de3f497474c86a41f2cf36d71f95e5ca06b074867b182a7047602c0000800000008000000080000000000c00000000", psbt.GetData().GetHex().c_str());
    EXPECT_STREQ("cHNidP8BAF4CAAAAAWkv08JB0xYYGWvDa23k9riJEU5CDpvd+cu+Ux5aVE1UAAAAAAD/////ARBLzR0AAAAAIgAgPK0GGd5n5iR6dqECgTY1wFNFfGuk/eSsH/2BSNcOS8wAAAAAAAEBIAhYzR0AAAAAF6kUlF+1A5GnBjfB/8Wrf7ZTCMLyMXWHAQMEAQAAAAEEIgAgnE2ssl67itqLuxrduGnepNgXDMlR8dlpSyFU4VgydskBBUdSIQOlEvX1nA55AfxHjtY1Pu929E+cssGFv7z38Lm7dnFr8yED9NRzYU6VSsT1UY5vfLMwe0+EdD4TftTuy19PtkPCO0dSriIGA6US9fWcDnkB/EeO1jU+73b0T5yywYW/vPfwubt2cWvzGCpwR2AsAACAAAAAgAAAAIAAAAAACwAAACIGA/TUc2FOlUrE9VGOb3yzMHtPhHQ+E37U7stfT7ZDwjtHGJ1rbYYsAACAAAAAgAAAAIAAAAAACwAAAAABAUdSIQKQbTmfbbvsyJjUuN4/SXR0yGpB8s821x+V5coGsHSGeyECYix5dOwy3nWvo3M7CabDPQ3qUUsp+qzPsJkXdPRiIkJSriICAmIseXTsMt51r6NzOwmmwz0N6lFLKfqsz7CZF3T0YiJCGJ1rbYYsAACAAAAAgAAAAIAAAAAADAAAACICApBtOZ9tu+zImNS43j9JdHTIakHyzzbXH5XlygawdIZ7GCpwR2AsAACAAAAAgAAAAIAAAAAADAAAAAA=", psbt.GetBase64().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  try {
    psbt1 = psbt;
    psbt1.Sign(key_in1.GetPrivkey());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  try {
    psbt2 = psbt;
    // psbt2.Sign(key_in2.GetPrivkey());
    // Same processing.
    auto sighash_type = psbt.GetTxInSighashType(0);
    auto utxo_txout = psbt.GetTxInUtxo(0);
    auto tx = psbt.GetTransaction();
    auto sighash = tx.GetSignatureHash(0, psbt.GetTxInRedeemScript(0).GetData(),
        sighash_type, utxo_txout.GetValue(), WitnessVersion::kVersion0);
    auto sig = key_in2.GetPrivkey().CalculateEcSignature(sighash);
    sig = CryptoUtil::ConvertSignatureToDer(sig, sighash_type);
    psbt2.SetTxInSignature(0, key_in2, sig);
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  try {
    psbt.Combine(psbt1);
    psbt.Combine(psbt2);
    EXPECT_STREQ("70736274ff01005e0200000001692fd3c241d31618196bc36b6de4f6b889114e420e9bddf9cbbe531e5a544d540000000000ffffffff01104bcd1d000000002200203cad0619de67e6247a76a102813635c053457c6ba4fde4ac1ffd8148d70e4bcc00000000000101200858cd1d0000000017a914945fb50391a70637c1ffc5ab7fb65308c2f2317587220203a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf3473044022020635937e05170d83dc3213adb3a6eae66713008e4abc38b4912c5680dec4f0c022033c08a71a30757b08463d530ec058ed7401968fb30bef63d4549721a0e485b8401220203f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b4747304402205d274a7887de3efade84a4a349c4c36f589b55623b1027b7da6dac89c178563402206a03afebbbb6089f1e37fac8f999a4015161c8920144fd53112da05804cd43cc010103040100000001042200209c4dacb25ebb8ada8bbb1addb869dea4d8170cc951f1d9694b2154e1583276c9010547522103a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf32103f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b4752ae220603a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf3182a7047602c0000800000008000000080000000000b000000220603f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b47189d6b6d862c0000800000008000000080000000000b00000000010147522102906d399f6dbbecc898d4b8de3f497474c86a41f2cf36d71f95e5ca06b074867b2102622c7974ec32de75afa3733b09a6c33d0dea514b29faaccfb0991774f462224252ae220202622c7974ec32de75afa3733b09a6c33d0dea514b29faaccfb0991774f4622242189d6b6d862c0000800000008000000080000000000c000000220202906d399f6dbbecc898d4b8de3f497474c86a41f2cf36d71f95e5ca06b074867b182a7047602c0000800000008000000080000000000c00000000", psbt.GetData().GetHex().c_str());
    EXPECT_STREQ("cHNidP8BAF4CAAAAAWkv08JB0xYYGWvDa23k9riJEU5CDpvd+cu+Ux5aVE1UAAAAAAD/////ARBLzR0AAAAAIgAgPK0GGd5n5iR6dqECgTY1wFNFfGuk/eSsH/2BSNcOS8wAAAAAAAEBIAhYzR0AAAAAF6kUlF+1A5GnBjfB/8Wrf7ZTCMLyMXWHIgIDpRL19ZwOeQH8R47WNT7vdvRPnLLBhb+89/C5u3Zxa/NHMEQCICBjWTfgUXDYPcMhOts6bq5mcTAI5KvDi0kSxWgN7E8MAiAzwIpxowdXsIRj1TDsBY7XQBlo+zC+9j1FSXIaDkhbhAEiAgP01HNhTpVKxPVRjm98szB7T4R0PhN+1O7LX0+2Q8I7R0cwRAIgXSdKeIfePvrehKSjScTDb1ibVWI7ECe32m2sicF4VjQCIGoDr+u7tgifHjf6yPmZpAFRYciSAUT9UxEtoFgEzUPMAQEDBAEAAAABBCIAIJxNrLJeu4rai7sa3bhp3qTYFwzJUfHZaUshVOFYMnbJAQVHUiEDpRL19ZwOeQH8R47WNT7vdvRPnLLBhb+89/C5u3Zxa/MhA/TUc2FOlUrE9VGOb3yzMHtPhHQ+E37U7stfT7ZDwjtHUq4iBgOlEvX1nA55AfxHjtY1Pu929E+cssGFv7z38Lm7dnFr8xgqcEdgLAAAgAAAAIAAAACAAAAAAAsAAAAiBgP01HNhTpVKxPVRjm98szB7T4R0PhN+1O7LX0+2Q8I7Rxida22GLAAAgAAAAIAAAACAAAAAAAsAAAAAAQFHUiECkG05n2277MiY1LjeP0l0dMhqQfLPNtcfleXKBrB0hnshAmIseXTsMt51r6NzOwmmwz0N6lFLKfqsz7CZF3T0YiJCUq4iAgJiLHl07DLeda+jczsJpsM9DepRSyn6rM+wmRd09GIiQhida22GLAAAgAAAAIAAAACAAAAAAAwAAAAiAgKQbTmfbbvsyJjUuN4/SXR0yGpB8s821x+V5coGsHSGexgqcEdgLAAAgAAAAIAAAACAAAAAAAwAAAAA", psbt.GetBase64().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  try {
    auto sig1 = psbt.GetTxInSignature(0, key_in1.GetPubkey());
    auto sig2 = psbt.GetTxInSignature(0, key_in2.GetPubkey());
    auto script = psbt.GetTxInRedeemScript(0);
    EXPECT_FALSE(psbt.IsFinalizedInput(0));
    EXPECT_FALSE(psbt.IsFinalized());
    psbt.SetTxInFinalScript(
        0, std::vector<ByteData>{ByteData(), sig1, sig2, script.GetData()});
    // psbt.Finalize();
    EXPECT_TRUE(psbt.IsFinalizedInput(0));
    psbt.ClearTxInSignData(0);

    auto wit_script = psbt.GetTxInFinalScript(0, true);
    auto wsh_script = psbt.GetTxInFinalScript(0, false);
    EXPECT_EQ(4, wit_script.size());
    if (4 == wit_script.size()) {
      EXPECT_EQ(script.GetHex(), wit_script[3].GetHex());
    }
    EXPECT_EQ(1, wsh_script.size());
    if (1 == wsh_script.size()) {
      auto exp_script = ScriptUtil::CreateP2wshLockingScript(script);
      EXPECT_EQ(exp_script.GetData().Serialize().GetHex(), wsh_script[0].GetHex());
    }
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  try {
    EXPECT_TRUE(psbt.IsFinalized());
    auto tx = psbt.ExtractTransaction();
    EXPECT_STREQ("02000000000101692fd3c241d31618196bc36b6de4f6b889114e420e9bddf9cbbe531e5a544d5400000000232200209c4dacb25ebb8ada8bbb1addb869dea4d8170cc951f1d9694b2154e1583276c9ffffffff01104bcd1d000000002200203cad0619de67e6247a76a102813635c053457c6ba4fde4ac1ffd8148d70e4bcc0400473044022020635937e05170d83dc3213adb3a6eae66713008e4abc38b4912c5680dec4f0c022033c08a71a30757b08463d530ec058ed7401968fb30bef63d4549721a0e485b840147304402205d274a7887de3efade84a4a349c4c36f589b55623b1027b7da6dac89c178563402206a03afebbbb6089f1e37fac8f999a4015161c8920144fd53112da05804cd43cc0147522103a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf32103f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b4752ae00000000", tx.GetHex().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }
}


TEST(Psbt, EmptyObject) {
  try {
    Psbt empty_obj;
    EXPECT_EQ(2, empty_obj.GetTransaction().GetVersion());
    EXPECT_EQ(0, empty_obj.GetTransaction().GetLockTime());
    EXPECT_EQ(19, empty_obj.GetDataSize());
    EXPECT_STREQ("70736274ff01000a0200000000000000000000",
        empty_obj.GetData().GetHex().c_str());
    EXPECT_STREQ("cHNidP8BAAoCAAAAAAAAAAAAAA==", empty_obj.GetBase64().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
  }

  try {
    Psbt obj(1, 100);
    EXPECT_EQ(1, obj.GetTransaction().GetVersion());
    EXPECT_EQ(100, obj.GetTransaction().GetLockTime());
    EXPECT_EQ(19, obj.GetDataSize());
    EXPECT_STREQ("70736274ff01000a0100000000006400000000",
        obj.GetData().GetHex().c_str());
    EXPECT_STREQ("cHNidP8BAAoBAAAAAABkAAAAAA==", obj.GetBase64().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
  }

  try {
    Psbt from_b64("cHNidP8BAAoBAAAAAABkAAAAAA==");
    EXPECT_EQ(1, from_b64.GetTransaction().GetVersion());
    EXPECT_EQ(100, from_b64.GetTransaction().GetLockTime());
    EXPECT_EQ(19, from_b64.GetDataSize());
    EXPECT_STREQ("70736274ff01000a0100000000006400000000",
        from_b64.GetData().GetHex().c_str());
    EXPECT_STREQ("cHNidP8BAAoBAAAAAABkAAAAAA==", from_b64.GetBase64().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
  }
}

struct CfdPsbtTestData {
  std::string hex;
  std::string base64;
  std::string error_message;
};

// https://github.com/bitcoin/bips/blob/master/bip-0174.mediawiki
static const std::vector<CfdPsbtTestData> g_cfd_psbt_testdata = {
  {
    "0200000001268171371edff285e937adeea4b37b78000c0566cbb3ad64641713ca42171bf6000000006a473044022070b2245123e6bf474d60c5b50c043d4c691a5d2435f09a34a7662a9dc251790a022001329ca9dacf280bdf30740ec0390422422c81cb45839457aeb76fc12edd95b3012102657d118d3357b8e0f4c2cd46db7b39f6d9c38d9a70abcb9b2de5dc8dbfe4ce31feffffff02d3dff505000000001976a914d0c59903c5bac2868760e90fd521a4665aa7652088ac00e1f5050000000017a9143545e6e33b832c47050f24d3eeb93c9c03948bc787b32e1300",
    "AgAAAAEmgXE3Ht/yhek3re6ks3t4AAwFZsuzrWRkFxPKQhcb9gAAAABqRzBEAiBwsiRRI+a/R01gxbUMBD1MaRpdJDXwmjSnZiqdwlF5CgIgATKcqdrPKAvfMHQOwDkEIkIsgctFg5RXrrdvwS7dlbMBIQJlfRGNM1e44PTCzUbbezn22cONmnCry5st5dyNv+TOMf7///8C09/1BQAAAAAZdqkU0MWZA8W6woaHYOkP1SGkZlqnZSCIrADh9QUAAAAAF6kUNUXm4zuDLEcFDyTT7rk8nAOUi8eHsy4TAA==",
    "psbt unmatch magic error."
  },
  {
    "70736274ff0100750200000001268171371edff285e937adeea4b37b78000c0566cbb3ad64641713ca42171bf60000000000feffffff02d3dff505000000001976a914d0c59903c5bac2868760e90fd521a4665aa7652088ac00e1f5050000000017a9143545e6e33b832c47050f24d3eeb93c9c03948bc787b32e1300000100fda5010100000000010289a3c71eab4d20e0371bbba4cc698fa295c9463afa2e397f8533ccb62f9567e50100000017160014be18d152a9b012039daf3da7de4f53349eecb985ffffffff86f8aa43a71dff1448893a530a7237ef6b4608bbb2dd2d0171e63aec6a4890b40100000017160014fe3e9ef1a745e974d902c4355943abcb34bd5353ffffffff0200c2eb0b000000001976a91485cff1097fd9e008bb34af709c62197b38978a4888ac72fef84e2c00000017a914339725ba21efd62ac753a9bcd067d6c7a6a39d05870247304402202712be22e0270f394f568311dc7ca9a68970b8025fdd3b240229f07f8a5f3a240220018b38d7dcd314e734c9276bd6fb40f673325bc4baa144c800d2f2f02db2765c012103d2e15674941bad4a996372cb87e1856d3652606d98562fe39c5e9e7e413f210502483045022100d12b852d85dcd961d2f5f4ab660654df6eedcc794c0c33ce5cc309ffb5fce58d022067338a8e0e1725c197fb1a88af59f51e44e4255b20167c8684031c05d1f2592a01210223b72beef0965d10be0778efecd61fcac6f79a4ea169393380734464f84f2ab30000000000",
    "cHNidP8BAHUCAAAAASaBcTce3/KF6Tet7qSze3gADAVmy7OtZGQXE8pCFxv2AAAAAAD+////AtPf9QUAAAAAGXapFNDFmQPFusKGh2DpD9UhpGZap2UgiKwA4fUFAAAAABepFDVF5uM7gyxHBQ8k0+65PJwDlIvHh7MuEwAAAQD9pQEBAAAAAAECiaPHHqtNIOA3G7ukzGmPopXJRjr6Ljl/hTPMti+VZ+UBAAAAFxYAFL4Y0VKpsBIDna89p95PUzSe7LmF/////4b4qkOnHf8USIk6UwpyN+9rRgi7st0tAXHmOuxqSJC0AQAAABcWABT+Pp7xp0XpdNkCxDVZQ6vLNL1TU/////8CAMLrCwAAAAAZdqkUhc/xCX/Z4Ai7NK9wnGIZeziXikiIrHL++E4sAAAAF6kUM5cluiHv1irHU6m80GfWx6ajnQWHAkcwRAIgJxK+IuAnDzlPVoMR3HyppolwuAJf3TskAinwf4pfOiQCIAGLONfc0xTnNMkna9b7QPZzMlvEuqFEyADS8vAtsnZcASED0uFWdJQbrUqZY3LLh+GFbTZSYG2YVi/jnF6efkE/IQUCSDBFAiEA0SuFLYXc2WHS9fSrZgZU327tzHlMDDPOXMMJ/7X85Y0CIGczio4OFyXBl/saiK9Z9R5E5CVbIBZ8hoQDHAXR8lkqASECI7cr7vCWXRC+B3jv7NYfysb3mk6haTkzgHNEZPhPKrMAAAAAAA==",
    "psbt format error."
  },
  {
    "70736274ff0100fd0a010200000002ab0949a08c5af7c49b8212f417e2f15ab3f5c33dcf153821a8139f877a5b7be4000000006a47304402204759661797c01b036b25928948686218347d89864b719e1f7fcf57d1e511658702205309eabf56aa4d8891ffd111fdf1336f3a29da866d7f8486d75546ceedaf93190121035cdc61fc7ba971c0b501a646a2a83b102cb43881217ca682dc86e2d73fa88292feffffffab0949a08c5af7c49b8212f417e2f15ab3f5c33dcf153821a8139f877a5b7be40100000000feffffff02603bea0b000000001976a914768a40bbd740cbe81d988e71de2a4d5c71396b1d88ac8e240000000000001976a9146f4620b553fa095e721b9ee0efe9fa039cca459788ac00000000000001012000e1f5050000000017a9143545e6e33b832c47050f24d3eeb93c9c03948bc787010416001485d13537f2e265405a34dbafa9e3dda01fb82308000000",
    "cHNidP8BAP0KAQIAAAACqwlJoIxa98SbghL0F+LxWrP1wz3PFTghqBOfh3pbe+QAAAAAakcwRAIgR1lmF5fAGwNrJZKJSGhiGDR9iYZLcZ4ff89X0eURZYcCIFMJ6r9Wqk2Ikf/REf3xM286KdqGbX+EhtdVRs7tr5MZASEDXNxh/HupccC1AaZGoqg7ECy0OIEhfKaC3Ibi1z+ogpL+////qwlJoIxa98SbghL0F+LxWrP1wz3PFTghqBOfh3pbe+QBAAAAAP7///8CYDvqCwAAAAAZdqkUdopAu9dAy+gdmI5x3ipNXHE5ax2IrI4kAAAAAAAAGXapFG9GILVT+glechue4O/p+gOcykWXiKwAAAAAAAABASAA4fUFAAAAABepFDVF5uM7gyxHBQ8k0+65PJwDlIvHhwEEFgAUhdE1N/LiZUBaNNuvqePdoB+4IwgAAAA=",
    "psbt format error."
  },
  {
    "70736274ff000100fda5010100000000010289a3c71eab4d20e0371bbba4cc698fa295c9463afa2e397f8533ccb62f9567e50100000017160014be18d152a9b012039daf3da7de4f53349eecb985ffffffff86f8aa43a71dff1448893a530a7237ef6b4608bbb2dd2d0171e63aec6a4890b40100000017160014fe3e9ef1a745e974d902c4355943abcb34bd5353ffffffff0200c2eb0b000000001976a91485cff1097fd9e008bb34af709c62197b38978a4888ac72fef84e2c00000017a914339725ba21efd62ac753a9bcd067d6c7a6a39d05870247304402202712be22e0270f394f568311dc7ca9a68970b8025fdd3b240229f07f8a5f3a240220018b38d7dcd314e734c9276bd6fb40f673325bc4baa144c800d2f2f02db2765c012103d2e15674941bad4a996372cb87e1856d3652606d98562fe39c5e9e7e413f210502483045022100d12b852d85dcd961d2f5f4ab660654df6eedcc794c0c33ce5cc309ffb5fce58d022067338a8e0e1725c197fb1a88af59f51e44e4255b20167c8684031c05d1f2592a01210223b72beef0965d10be0778efecd61fcac6f79a4ea169393380734464f84f2ab30000000000",
    "cHNidP8AAQD9pQEBAAAAAAECiaPHHqtNIOA3G7ukzGmPopXJRjr6Ljl/hTPMti+VZ+UBAAAAFxYAFL4Y0VKpsBIDna89p95PUzSe7LmF/////4b4qkOnHf8USIk6UwpyN+9rRgi7st0tAXHmOuxqSJC0AQAAABcWABT+Pp7xp0XpdNkCxDVZQ6vLNL1TU/////8CAMLrCwAAAAAZdqkUhc/xCX/Z4Ai7NK9wnGIZeziXikiIrHL++E4sAAAAF6kUM5cluiHv1irHU6m80GfWx6ajnQWHAkcwRAIgJxK+IuAnDzlPVoMR3HyppolwuAJf3TskAinwf4pfOiQCIAGLONfc0xTnNMkna9b7QPZzMlvEuqFEyADS8vAtsnZcASED0uFWdJQbrUqZY3LLh+GFbTZSYG2YVi/jnF6efkE/IQUCSDBFAiEA0SuFLYXc2WHS9fSrZgZU327tzHlMDDPOXMMJ/7X85Y0CIGczio4OFyXBl/saiK9Z9R5E5CVbIBZ8hoQDHAXR8lkqASECI7cr7vCWXRC+B3jv7NYfysb3mk6haTkzgHNEZPhPKrMAAAAAAA==",
    "psbt global tx not found error."
  },
  {
    "70736274ff0100750200000001268171371edff285e937adeea4b37b78000c0566cbb3ad64641713ca42171bf60000000000feffffff02d3dff505000000001976a914d0c59903c5bac2868760e90fd521a4665aa7652088ac00e1f5050000000017a9143545e6e33b832c47050f24d3eeb93c9c03948bc787b32e1300000100fda5010100000000010289a3c71eab4d20e0371bbba4cc698fa295c9463afa2e397f8533ccb62f9567e50100000017160014be18d152a9b012039daf3da7de4f53349eecb985ffffffff86f8aa43a71dff1448893a530a7237ef6b4608bbb2dd2d0171e63aec6a4890b40100000017160014fe3e9ef1a745e974d902c4355943abcb34bd5353ffffffff0200c2eb0b000000001976a91485cff1097fd9e008bb34af709c62197b38978a4888ac72fef84e2c00000017a914339725ba21efd62ac753a9bcd067d6c7a6a39d05870247304402202712be22e0270f394f568311dc7ca9a68970b8025fdd3b240229f07f8a5f3a240220018b38d7dcd314e734c9276bd6fb40f673325bc4baa144c800d2f2f02db2765c012103d2e15674941bad4a996372cb87e1856d3652606d98562fe39c5e9e7e413f210502483045022100d12b852d85dcd961d2f5f4ab660654df6eedcc794c0c33ce5cc309ffb5fce58d022067338a8e0e1725c197fb1a88af59f51e44e4255b20167c8684031c05d1f2592a01210223b72beef0965d10be0778efecd61fcac6f79a4ea169393380734464f84f2ab30000000001003f0200000001ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff0000000000ffffffff010000000000000000036a010000000000000000",
    "cHNidP8BAHUCAAAAASaBcTce3/KF6Tet7qSze3gADAVmy7OtZGQXE8pCFxv2AAAAAAD+////AtPf9QUAAAAAGXapFNDFmQPFusKGh2DpD9UhpGZap2UgiKwA4fUFAAAAABepFDVF5uM7gyxHBQ8k0+65PJwDlIvHh7MuEwAAAQD9pQEBAAAAAAECiaPHHqtNIOA3G7ukzGmPopXJRjr6Ljl/hTPMti+VZ+UBAAAAFxYAFL4Y0VKpsBIDna89p95PUzSe7LmF/////4b4qkOnHf8USIk6UwpyN+9rRgi7st0tAXHmOuxqSJC0AQAAABcWABT+Pp7xp0XpdNkCxDVZQ6vLNL1TU/////8CAMLrCwAAAAAZdqkUhc/xCX/Z4Ai7NK9wnGIZeziXikiIrHL++E4sAAAAF6kUM5cluiHv1irHU6m80GfWx6ajnQWHAkcwRAIgJxK+IuAnDzlPVoMR3HyppolwuAJf3TskAinwf4pfOiQCIAGLONfc0xTnNMkna9b7QPZzMlvEuqFEyADS8vAtsnZcASED0uFWdJQbrUqZY3LLh+GFbTZSYG2YVi/jnF6efkE/IQUCSDBFAiEA0SuFLYXc2WHS9fSrZgZU327tzHlMDDPOXMMJ/7X85Y0CIGczio4OFyXBl/saiK9Z9R5E5CVbIBZ8hoQDHAXR8lkqASECI7cr7vCWXRC+B3jv7NYfysb3mk6haTkzgHNEZPhPKrMAAAAAAQA/AgAAAAH//////////////////////////////////////////wAAAAAA/////wEAAAAAAAAAAANqAQAAAAAAAAAA",
    "psbt format error."
  },
  {
    "70736274ff020001550200000001279a2323a5dfb51fc45f220fa58b0fc13e1e3342792a85d7e36cd6333b5cbc390000000000ffffffff01a05aea0b000000001976a914ffe9c0061097cc3b636f2cb0460fa4fc427d2b4588ac0000000000010120955eea0b0000000017a9146345200f68d189e1adc0df1c4d16ea8f14c0dbeb87220203b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd4646304302200424b58effaaa694e1559ea5c93bbfd4a89064224055cdf070b6771469442d07021f5c8eb0fea6516d60b8acb33ad64ede60e8785bfb3aa94b99bdf86151db9a9a010104220020771fd18ad459666dd49f3d564e3dbc42f4c84774e360ada16816a8ed488d5681010547522103b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd462103de55d1e1dac805e3f8a58c1fbf9b94c02f3dbaafe127fefca4995f26f82083bd52ae220603b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd4610b4a6ba67000000800000008004000080220603de55d1e1dac805e3f8a58c1fbf9b94c02f3dbaafe127fefca4995f26f82083bd10b4a6ba670000008000000080050000800000",
    "cHNidP8CAAFVAgAAAAEnmiMjpd+1H8RfIg+liw/BPh4zQnkqhdfjbNYzO1y8OQAAAAAA/////wGgWuoLAAAAABl2qRT/6cAGEJfMO2NvLLBGD6T8Qn0rRYisAAAAAAABASCVXuoLAAAAABepFGNFIA9o0YnhrcDfHE0W6o8UwNvrhyICA7E0HMunaDtq9PEjjNbpfnFn1Wn6xH8eSNR1QYRDVb1GRjBDAiAEJLWO/6qmlOFVnqXJO7/UqJBkIkBVzfBwtncUaUQtBwIfXI6w/qZRbWC4rLM61k7eYOh4W/s6qUuZvfhhUduamgEBBCIAIHcf0YrUWWZt1J89Vk49vEL0yEd042CtoWgWqO1IjVaBAQVHUiEDsTQcy6doO2r08SOM1ul+cWfVafrEfx5I1HVBhENVvUYhA95V0eHayAXj+KWMH7+blMAvPbqv4Sf+/KSZXyb4IIO9Uq4iBgOxNBzLp2g7avTxI4zW6X5xZ9Vp+sR/HkjUdUGEQ1W9RhC0prpnAAAAgAAAAIAEAACAIgYD3lXR4drIBeP4pYwfv5uUwC89uq/hJ/78pJlfJvggg70QtKa6ZwAAAIAAAACABQAAgAAA",
    "psbt invalid key format error."
  },
  {
    "70736274ff0100550200000001279a2323a5dfb51fc45f220fa58b0fc13e1e3342792a85d7e36cd6333b5cbc390000000000ffffffff01a05aea0b000000001976a914ffe9c0061097cc3b636f2cb0460fa4fc427d2b4588ac000000000002010020955eea0b0000000017a9146345200f68d189e1adc0df1c4d16ea8f14c0dbeb87220203b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd4646304302200424b58effaaa694e1559ea5c93bbfd4a89064224055cdf070b6771469442d07021f5c8eb0fea6516d60b8acb33ad64ede60e8785bfb3aa94b99bdf86151db9a9a010104220020771fd18ad459666dd49f3d564e3dbc42f4c84774e360ada16816a8ed488d5681010547522103b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd462103de55d1e1dac805e3f8a58c1fbf9b94c02f3dbaafe127fefca4995f26f82083bd52ae220603b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd4610b4a6ba67000000800000008004000080220603de55d1e1dac805e3f8a58c1fbf9b94c02f3dbaafe127fefca4995f26f82083bd10b4a6ba670000008000000080050000800000",
    "cHNidP8BAFUCAAAAASeaIyOl37UfxF8iD6WLD8E+HjNCeSqF1+Ns1jM7XLw5AAAAAAD/////AaBa6gsAAAAAGXapFP/pwAYQl8w7Y28ssEYPpPxCfStFiKwAAAAAAAIBACCVXuoLAAAAABepFGNFIA9o0YnhrcDfHE0W6o8UwNvrhyICA7E0HMunaDtq9PEjjNbpfnFn1Wn6xH8eSNR1QYRDVb1GRjBDAiAEJLWO/6qmlOFVnqXJO7/UqJBkIkBVzfBwtncUaUQtBwIfXI6w/qZRbWC4rLM61k7eYOh4W/s6qUuZvfhhUduamgEBBCIAIHcf0YrUWWZt1J89Vk49vEL0yEd042CtoWgWqO1IjVaBAQVHUiEDsTQcy6doO2r08SOM1ul+cWfVafrEfx5I1HVBhENVvUYhA95V0eHayAXj+KWMH7+blMAvPbqv4Sf+/KSZXyb4IIO9Uq4iBgOxNBzLp2g7avTxI4zW6X5xZ9Vp+sR/HkjUdUGEQ1W9RhC0prpnAAAAgAAAAIAEAACAIgYD3lXR4drIBeP4pYwfv5uUwC89uq/hJ/78pJlfJvggg70QtKa6ZwAAAIAAAACABQAAgAAA",
    "psbt format error."
  },
  {
    "70736274ff0100550200000001279a2323a5dfb51fc45f220fa58b0fc13e1e3342792a85d7e36cd6333b5cbc390000000000ffffffff01a05aea0b000000001976a914ffe9c0061097cc3b636f2cb0460fa4fc427d2b4588ac0000000000010120955eea0b0000000017a9146345200f68d189e1adc0df1c4d16ea8f14c0dbeb87210203b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd46304302200424b58effaaa694e1559ea5c93bbfd4a89064224055cdf070b6771469442d07021f5c8eb0fea6516d60b8acb33ad64ede60e8785bfb3aa94b99bdf86151db9a9a010104220020771fd18ad459666dd49f3d564e3dbc42f4c84774e360ada16816a8ed488d5681010547522103b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd462103de55d1e1dac805e3f8a58c1fbf9b94c02f3dbaafe127fefca4995f26f82083bd52ae220603b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd4610b4a6ba67000000800000008004000080220603de55d1e1dac805e3f8a58c1fbf9b94c02f3dbaafe127fefca4995f26f82083bd10b4a6ba670000008000000080050000800000",
    "cHNidP8BAFUCAAAAASeaIyOl37UfxF8iD6WLD8E+HjNCeSqF1+Ns1jM7XLw5AAAAAAD/////AaBa6gsAAAAAGXapFP/pwAYQl8w7Y28ssEYPpPxCfStFiKwAAAAAAAEBIJVe6gsAAAAAF6kUY0UgD2jRieGtwN8cTRbqjxTA2+uHIQIDsTQcy6doO2r08SOM1ul+cWfVafrEfx5I1HVBhENVvUYwQwIgBCS1jv+qppThVZ6lyTu/1KiQZCJAVc3wcLZ3FGlELQcCH1yOsP6mUW1guKyzOtZO3mDoeFv7OqlLmb34YVHbmpoBAQQiACB3H9GK1FlmbdSfPVZOPbxC9MhHdONgraFoFqjtSI1WgQEFR1IhA7E0HMunaDtq9PEjjNbpfnFn1Wn6xH8eSNR1QYRDVb1GIQPeVdHh2sgF4/iljB+/m5TALz26r+En/vykmV8m+CCDvVKuIgYDsTQcy6doO2r08SOM1ul+cWfVafrEfx5I1HVBhENVvUYQtKa6ZwAAAIAAAACABAAAgCIGA95V0eHayAXj+KWMH7+blMAvPbqv4Sf+/KSZXyb4IIO9ELSmumcAAACAAAAAgAUAAIAAAA==",
    "psbt format error."
  },
  {
    "70736274ff0100550200000001279a2323a5dfb51fc45f220fa58b0fc13e1e3342792a85d7e36cd6333b5cbc390000000000ffffffff01a05aea0b000000001976a914ffe9c0061097cc3b636f2cb0460fa4fc427d2b4588ac0000000000010120955eea0b0000000017a9146345200f68d189e1adc0df1c4d16ea8f14c0dbeb87220203b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd4646304302200424b58effaaa694e1559ea5c93bbfd4a89064224055cdf070b6771469442d07021f5c8eb0fea6516d60b8acb33ad64ede60e8785bfb3aa94b99bdf86151db9a9a01020400220020771fd18ad459666dd49f3d564e3dbc42f4c84774e360ada16816a8ed488d5681010547522103b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd462103de55d1e1dac805e3f8a58c1fbf9b94c02f3dbaafe127fefca4995f26f82083bd52ae220603b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd4610b4a6ba67000000800000008004000080220603de55d1e1dac805e3f8a58c1fbf9b94c02f3dbaafe127fefca4995f26f82083bd10b4a6ba670000008000000080050000800000",
    "cHNidP8BAFUCAAAAASeaIyOl37UfxF8iD6WLD8E+HjNCeSqF1+Ns1jM7XLw5AAAAAAD/////AaBa6gsAAAAAGXapFP/pwAYQl8w7Y28ssEYPpPxCfStFiKwAAAAAAAEBIJVe6gsAAAAAF6kUY0UgD2jRieGtwN8cTRbqjxTA2+uHIgIDsTQcy6doO2r08SOM1ul+cWfVafrEfx5I1HVBhENVvUZGMEMCIAQktY7/qqaU4VWepck7v9SokGQiQFXN8HC2dxRpRC0HAh9cjrD+plFtYLisszrWTt5g6Hhb+zqpS5m9+GFR25qaAQIEACIAIHcf0YrUWWZt1J89Vk49vEL0yEd042CtoWgWqO1IjVaBAQVHUiEDsTQcy6doO2r08SOM1ul+cWfVafrEfx5I1HVBhENVvUYhA95V0eHayAXj+KWMH7+blMAvPbqv4Sf+/KSZXyb4IIO9Uq4iBgOxNBzLp2g7avTxI4zW6X5xZ9Vp+sR/HkjUdUGEQ1W9RhC0prpnAAAAgAAAAIAEAACAIgYD3lXR4drIBeP4pYwfv5uUwC89uq/hJ/78pJlfJvggg70QtKa6ZwAAAIAAAACABQAAgAAA",
    "psbt format error."
  },
  {
    "70736274ff0100550200000001279a2323a5dfb51fc45f220fa58b0fc13e1e3342792a85d7e36cd6333b5cbc390000000000ffffffff01a05aea0b000000001976a914ffe9c0061097cc3b636f2cb0460fa4fc427d2b4588ac0000000000010120955eea0b0000000017a9146345200f68d189e1adc0df1c4d16ea8f14c0dbeb87220203b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd4646304302200424b58effaaa694e1559ea5c93bbfd4a89064224055cdf070b6771469442d07021f5c8eb0fea6516d60b8acb33ad64ede60e8785bfb3aa94b99bdf86151db9a9a010104220020771fd18ad459666dd49f3d564e3dbc42f4c84774e360ada16816a8ed488d568102050047522103b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd462103de55d1e1dac805e3f8a58c1fbf9b94c02f3dbaafe127fefca4995f26f82083bd52ae220603b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd4610b4a6ba67000000800000008004000080220603de55d1e1dac805e3f8a58c1fbf9b94c02f3dbaafe127fefca4995f26f82083bd10b4a6ba670000008000000080050000800000",
    "cHNidP8BAFUCAAAAASeaIyOl37UfxF8iD6WLD8E+HjNCeSqF1+Ns1jM7XLw5AAAAAAD/////AaBa6gsAAAAAGXapFP/pwAYQl8w7Y28ssEYPpPxCfStFiKwAAAAAAAEBIJVe6gsAAAAAF6kUY0UgD2jRieGtwN8cTRbqjxTA2+uHIgIDsTQcy6doO2r08SOM1ul+cWfVafrEfx5I1HVBhENVvUZGMEMCIAQktY7/qqaU4VWepck7v9SokGQiQFXN8HC2dxRpRC0HAh9cjrD+plFtYLisszrWTt5g6Hhb+zqpS5m9+GFR25qaAQEEIgAgdx/RitRZZm3Unz1WTj28QvTIR3TjYK2haBao7UiNVoECBQBHUiEDsTQcy6doO2r08SOM1ul+cWfVafrEfx5I1HVBhENVvUYhA95V0eHayAXj+KWMH7+blMAvPbqv4Sf+/KSZXyb4IIO9Uq4iBgOxNBzLp2g7avTxI4zW6X5xZ9Vp+sR/HkjUdUGEQ1W9RhC0prpnAAAAgAAAAIAEAACAIgYD3lXR4drIBeP4pYwfv5uUwC89uq/hJ/78pJlfJvggg70QtKa6ZwAAAIAAAACABQAAgAAA",
    "psbt format error."
  },
  {
    "70736274ff0100550200000001279a2323a5dfb51fc45f220fa58b0fc13e1e3342792a85d7e36cd6333b5cbc390000000000ffffffff01a05aea0b000000001976a914ffe9c0061097cc3b636f2cb0460fa4fc427d2b4588ac0000000000010120955eea0b0000000017a9146345200f68d189e1adc0df1c4d16ea8f14c0dbeb87220203b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd4646304302200424b58effaaa694e1559ea5c93bbfd4a89064224055cdf070b6771469442d07021f5c8eb0fea6516d60b8acb33ad64ede60e8785bfb3aa94b99bdf86151db9a9a010104220020771fd18ad459666dd49f3d564e3dbc42f4c84774e360ada16816a8ed488d5681010547522103b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd462103de55d1e1dac805e3f8a58c1fbf9b94c02f3dbaafe127fefca4995f26f82083bd52ae210603b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd10b4a6ba67000000800000008004000080220603de55d1e1dac805e3f8a58c1fbf9b94c02f3dbaafe127fefca4995f26f82083bd10b4a6ba670000008000000080050000800000",
    "cHNidP8BAFUCAAAAASeaIyOl37UfxF8iD6WLD8E+HjNCeSqF1+Ns1jM7XLw5AAAAAAD/////AaBa6gsAAAAAGXapFP/pwAYQl8w7Y28ssEYPpPxCfStFiKwAAAAAAAEBIJVe6gsAAAAAF6kUY0UgD2jRieGtwN8cTRbqjxTA2+uHIgIDsTQcy6doO2r08SOM1ul+cWfVafrEfx5I1HVBhENVvUZGMEMCIAQktY7/qqaU4VWepck7v9SokGQiQFXN8HC2dxRpRC0HAh9cjrD+plFtYLisszrWTt5g6Hhb+zqpS5m9+GFR25qaAQEEIgAgdx/RitRZZm3Unz1WTj28QvTIR3TjYK2haBao7UiNVoEBBUdSIQOxNBzLp2g7avTxI4zW6X5xZ9Vp+sR/HkjUdUGEQ1W9RiED3lXR4drIBeP4pYwfv5uUwC89uq/hJ/78pJlfJvggg71SriEGA7E0HMunaDtq9PEjjNbpfnFn1Wn6xH8eSNR1QYRDVb0QtKa6ZwAAAIAAAACABAAAgCIGA95V0eHayAXj+KWMH7+blMAvPbqv4Sf+/KSZXyb4IIO9ELSmumcAAACAAAAAgAUAAIAAAA==",
    "psbt format error."
  },
  {
    "70736274ff01009a020000000258e87a21b56daf0c23be8e7070456c336f7cbaa5c8757924f545887bb2abdd750000000000ffffffff838d0427d0ec650a68aa46bb0b098aea4422c071b2ca78352a077959d07cea1d0100000000ffffffff0270aaf00800000000160014d85c2b71d0060b09c9886aeb815e50991dda124d00e1f5050000000016001400aea9a2e5f0f876a588df5546e8742d1d87008f0000000000020000bb0200000001aad73931018bd25f84ae400b68848be09db706eac2ac18298babee71ab656f8b0000000048473044022058f6fc7c6a33e1b31548d481c826c015bd30135aad42cd67790dab66d2ad243b02204a1ced2604c6735b6393e5b41691dd78b00f0c5942fb9f751856faa938157dba01feffffff0280f0fa020000000017a9140fb9463421696b82c833af241c78c17ddbde493487d0f20a270100000017a91429ca74f8a08f81999428185c97b5d852e4063f6187650000000107da00473044022074018ad4180097b873323c0015720b3684cc8123891048e7dbcd9b55ad679c99022073d369b740e3eb53dcefa33823c8070514ca55a7dd9544f157c167913261118c01483045022100f61038b308dc1da865a34852746f015772934208c6d24454393cd99bdf2217770220056e675a675a6d0a02b85b14e5e29074d8a25a9b5760bea2816f661910a006ea01475221029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f2102dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d752ae0001012000c2eb0b0000000017a914b7f5faf40e3d40a5a459b1db3535f2b72fa921e8870107232200208c2353173743b595dfb4a07b72ba8e42e3797da74e87fe7d9d7497e3b20289030108da0400473044022062eb7a556107a7c73f45ac4ab5a1dddf6f7075fb1275969a7f383efff784bcb202200c05dbb7470dbf2f08557dd356c7325c1ed30913e996cd3840945db12228da5f01473044022065f45ba5998b59a27ffe1a7bed016af1f1f90d54b3aa8f7450aa5f56a25103bd02207f724703ad1edb96680b284b56d4ffcb88f7fb759eabbe08aa30f29b851383d20147522103089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc21023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7352ae00220203a9a4c37f5996d3aa25dbac6b570af0650394492942460b354753ed9eeca5877110d90c6a4f000000800000008004000080002202027f6399757d2eff55a136ad02c684b1838b6556e5f1b6b34282a94b6b5005109610d90c6a4f00000080000000800500008000",
    "cHNidP8BAJoCAAAAAljoeiG1ba8MI76OcHBFbDNvfLqlyHV5JPVFiHuyq911AAAAAAD/////g40EJ9DsZQpoqka7CwmK6kQiwHGyyng1Kgd5WdB86h0BAAAAAP////8CcKrwCAAAAAAWABTYXCtx0AYLCcmIauuBXlCZHdoSTQDh9QUAAAAAFgAUAK6pouXw+HaliN9VRuh0LR2HAI8AAAAAAAIAALsCAAAAAarXOTEBi9JfhK5AC2iEi+CdtwbqwqwYKYur7nGrZW+LAAAAAEhHMEQCIFj2/HxqM+GzFUjUgcgmwBW9MBNarULNZ3kNq2bSrSQ7AiBKHO0mBMZzW2OT5bQWkd14sA8MWUL7n3UYVvqpOBV9ugH+////AoDw+gIAAAAAF6kUD7lGNCFpa4LIM68kHHjBfdveSTSH0PIKJwEAAAAXqRQpynT4oI+BmZQoGFyXtdhS5AY/YYdlAAAAAQfaAEcwRAIgdAGK1BgAl7hzMjwAFXILNoTMgSOJEEjn282bVa1nnJkCIHPTabdA4+tT3O+jOCPIBwUUylWn3ZVE8VfBZ5EyYRGMAUgwRQIhAPYQOLMI3B2oZaNIUnRvAVdyk0IIxtJEVDk82ZvfIhd3AiAFbmdaZ1ptCgK4WxTl4pB02KJam1dgvqKBb2YZEKAG6gFHUiEClYO/Oa4KYJdHrRma3dY0+mEIVZ1sXNObTCGD8auW4H8hAtq2H/SaFNtqfQKwzR+7ePxLGDErW05U2uTbovv+9TbXUq4AAQEgAMLrCwAAAAAXqRS39fr0Dj1ApaRZsds1NfK3L6kh6IcBByMiACCMI1MXN0O1ld+0oHtyuo5C43l9p06H/n2ddJfjsgKJAwEI2gQARzBEAiBi63pVYQenxz9FrEq1od3fb3B1+xJ1lpp/OD7/94S8sgIgDAXbt0cNvy8IVX3TVscyXB7TCRPpls04QJRdsSIo2l8BRzBEAiBl9FulmYtZon/+GnvtAWrx8fkNVLOqj3RQql9WolEDvQIgf3JHA60e25ZoCyhLVtT/y4j3+3Weq74IqjDym4UTg9IBR1IhAwidwQx6xttU+RMpr2FzM9s4jOrQwjH3IzedG5kDCwLcIQI63ZBPPW3PWd25BrDe4jUpt/+57VDl6GFRkmhgIh8Oc1KuACICA6mkw39ZltOqJdusa1cK8GUDlEkpQkYLNUdT7Z7spYdxENkMak8AAACAAAAAgAQAAIAAIgICf2OZdX0u/1WhNq0CxoSxg4tlVuXxtrNCgqlLa1AFEJYQ2QxqTwAAAIAAAACABQAAgAA=",
    "psbt format error."
  },
  {
    "70736274ff01009a020000000258e87a21b56daf0c23be8e7070456c336f7cbaa5c8757924f545887bb2abdd750000000000ffffffff838d0427d0ec650a68aa46bb0b098aea4422c071b2ca78352a077959d07cea1d0100000000ffffffff0270aaf00800000000160014d85c2b71d0060b09c9886aeb815e50991dda124d00e1f5050000000016001400aea9a2e5f0f876a588df5546e8742d1d87008f00000000000100bb0200000001aad73931018bd25f84ae400b68848be09db706eac2ac18298babee71ab656f8b0000000048473044022058f6fc7c6a33e1b31548d481c826c015bd30135aad42cd67790dab66d2ad243b02204a1ced2604c6735b6393e5b41691dd78b00f0c5942fb9f751856faa938157dba01feffffff0280f0fa020000000017a9140fb9463421696b82c833af241c78c17ddbde493487d0f20a270100000017a91429ca74f8a08f81999428185c97b5d852e4063f618765000000020700da00473044022074018ad4180097b873323c0015720b3684cc8123891048e7dbcd9b55ad679c99022073d369b740e3eb53dcefa33823c8070514ca55a7dd9544f157c167913261118c01483045022100f61038b308dc1da865a34852746f015772934208c6d24454393cd99bdf2217770220056e675a675a6d0a02b85b14e5e29074d8a25a9b5760bea2816f661910a006ea01475221029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f2102dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d752ae0001012000c2eb0b0000000017a914b7f5faf40e3d40a5a459b1db3535f2b72fa921e8870107232200208c2353173743b595dfb4a07b72ba8e42e3797da74e87fe7d9d7497e3b20289030108da0400473044022062eb7a556107a7c73f45ac4ab5a1dddf6f7075fb1275969a7f383efff784bcb202200c05dbb7470dbf2f08557dd356c7325c1ed30913e996cd3840945db12228da5f01473044022065f45ba5998b59a27ffe1a7bed016af1f1f90d54b3aa8f7450aa5f56a25103bd02207f724703ad1edb96680b284b56d4ffcb88f7fb759eabbe08aa30f29b851383d20147522103089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc21023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7352ae00220203a9a4c37f5996d3aa25dbac6b570af0650394492942460b354753ed9eeca5877110d90c6a4f000000800000008004000080002202027f6399757d2eff55a136ad02c684b1838b6556e5f1b6b34282a94b6b5005109610d90c6a4f00000080000000800500008000",
    "cHNidP8BAJoCAAAAAljoeiG1ba8MI76OcHBFbDNvfLqlyHV5JPVFiHuyq911AAAAAAD/////g40EJ9DsZQpoqka7CwmK6kQiwHGyyng1Kgd5WdB86h0BAAAAAP////8CcKrwCAAAAAAWABTYXCtx0AYLCcmIauuBXlCZHdoSTQDh9QUAAAAAFgAUAK6pouXw+HaliN9VRuh0LR2HAI8AAAAAAAEAuwIAAAABqtc5MQGL0l+ErkALaISL4J23BurCrBgpi6vucatlb4sAAAAASEcwRAIgWPb8fGoz4bMVSNSByCbAFb0wE1qtQs1neQ2rZtKtJDsCIEoc7SYExnNbY5PltBaR3XiwDwxZQvufdRhW+qk4FX26Af7///8CgPD6AgAAAAAXqRQPuUY0IWlrgsgzryQceMF9295JNIfQ8gonAQAAABepFCnKdPigj4GZlCgYXJe12FLkBj9hh2UAAAACBwDaAEcwRAIgdAGK1BgAl7hzMjwAFXILNoTMgSOJEEjn282bVa1nnJkCIHPTabdA4+tT3O+jOCPIBwUUylWn3ZVE8VfBZ5EyYRGMAUgwRQIhAPYQOLMI3B2oZaNIUnRvAVdyk0IIxtJEVDk82ZvfIhd3AiAFbmdaZ1ptCgK4WxTl4pB02KJam1dgvqKBb2YZEKAG6gFHUiEClYO/Oa4KYJdHrRma3dY0+mEIVZ1sXNObTCGD8auW4H8hAtq2H/SaFNtqfQKwzR+7ePxLGDErW05U2uTbovv+9TbXUq4AAQEgAMLrCwAAAAAXqRS39fr0Dj1ApaRZsds1NfK3L6kh6IcBByMiACCMI1MXN0O1ld+0oHtyuo5C43l9p06H/n2ddJfjsgKJAwEI2gQARzBEAiBi63pVYQenxz9FrEq1od3fb3B1+xJ1lpp/OD7/94S8sgIgDAXbt0cNvy8IVX3TVscyXB7TCRPpls04QJRdsSIo2l8BRzBEAiBl9FulmYtZon/+GnvtAWrx8fkNVLOqj3RQql9WolEDvQIgf3JHA60e25ZoCyhLVtT/y4j3+3Weq74IqjDym4UTg9IBR1IhAwidwQx6xttU+RMpr2FzM9s4jOrQwjH3IzedG5kDCwLcIQI63ZBPPW3PWd25BrDe4jUpt/+57VDl6GFRkmhgIh8Oc1KuACICA6mkw39ZltOqJdusa1cK8GUDlEkpQkYLNUdT7Z7spYdxENkMak8AAACAAAAAgAQAAIAAIgICf2OZdX0u/1WhNq0CxoSxg4tlVuXxtrNCgqlLa1AFEJYQ2QxqTwAAAIAAAACABQAAgAA=",
    "psbt format error."
  },
  {
    "70736274ff01009a020000000258e87a21b56daf0c23be8e7070456c336f7cbaa5c8757924f545887bb2abdd750000000000ffffffff838d0427d0ec650a68aa46bb0b098aea4422c071b2ca78352a077959d07cea1d0100000000ffffffff0270aaf00800000000160014d85c2b71d0060b09c9886aeb815e50991dda124d00e1f5050000000016001400aea9a2e5f0f876a588df5546e8742d1d87008f00000000000100bb0200000001aad73931018bd25f84ae400b68848be09db706eac2ac18298babee71ab656f8b0000000048473044022058f6fc7c6a33e1b31548d481c826c015bd30135aad42cd67790dab66d2ad243b02204a1ced2604c6735b6393e5b41691dd78b00f0c5942fb9f751856faa938157dba01feffffff0280f0fa020000000017a9140fb9463421696b82c833af241c78c17ddbde493487d0f20a270100000017a91429ca74f8a08f81999428185c97b5d852e4063f6187650000000107da00473044022074018ad4180097b873323c0015720b3684cc8123891048e7dbcd9b55ad679c99022073d369b740e3eb53dcefa33823c8070514ca55a7dd9544f157c167913261118c01483045022100f61038b308dc1da865a34852746f015772934208c6d24454393cd99bdf2217770220056e675a675a6d0a02b85b14e5e29074d8a25a9b5760bea2816f661910a006ea01475221029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f2102dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d752ae0001012000c2eb0b0000000017a914b7f5faf40e3d40a5a459b1db3535f2b72fa921e8870107232200208c2353173743b595dfb4a07b72ba8e42e3797da74e87fe7d9d7497e3b2028903020800da0400473044022062eb7a556107a7c73f45ac4ab5a1dddf6f7075fb1275969a7f383efff784bcb202200c05dbb7470dbf2f08557dd356c7325c1ed30913e996cd3840945db12228da5f01473044022065f45ba5998b59a27ffe1a7bed016af1f1f90d54b3aa8f7450aa5f56a25103bd02207f724703ad1edb96680b284b56d4ffcb88f7fb759eabbe08aa30f29b851383d20147522103089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc21023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7352ae00220203a9a4c37f5996d3aa25dbac6b570af0650394492942460b354753ed9eeca5877110d90c6a4f000000800000008004000080002202027f6399757d2eff55a136ad02c684b1838b6556e5f1b6b34282a94b6b5005109610d90c6a4f00000080000000800500008000",
    "cHNidP8BAJoCAAAAAljoeiG1ba8MI76OcHBFbDNvfLqlyHV5JPVFiHuyq911AAAAAAD/////g40EJ9DsZQpoqka7CwmK6kQiwHGyyng1Kgd5WdB86h0BAAAAAP////8CcKrwCAAAAAAWABTYXCtx0AYLCcmIauuBXlCZHdoSTQDh9QUAAAAAFgAUAK6pouXw+HaliN9VRuh0LR2HAI8AAAAAAAEAuwIAAAABqtc5MQGL0l+ErkALaISL4J23BurCrBgpi6vucatlb4sAAAAASEcwRAIgWPb8fGoz4bMVSNSByCbAFb0wE1qtQs1neQ2rZtKtJDsCIEoc7SYExnNbY5PltBaR3XiwDwxZQvufdRhW+qk4FX26Af7///8CgPD6AgAAAAAXqRQPuUY0IWlrgsgzryQceMF9295JNIfQ8gonAQAAABepFCnKdPigj4GZlCgYXJe12FLkBj9hh2UAAAABB9oARzBEAiB0AYrUGACXuHMyPAAVcgs2hMyBI4kQSOfbzZtVrWecmQIgc9Npt0Dj61Pc76M4I8gHBRTKVafdlUTxV8FnkTJhEYwBSDBFAiEA9hA4swjcHahlo0hSdG8BV3KTQgjG0kRUOTzZm98iF3cCIAVuZ1pnWm0KArhbFOXikHTYolqbV2C+ooFvZhkQoAbqAUdSIQKVg785rgpgl0etGZrd1jT6YQhVnWxc05tMIYPxq5bgfyEC2rYf9JoU22p9ArDNH7t4/EsYMStbTlTa5Nui+/71NtdSrgABASAAwusLAAAAABepFLf1+vQOPUClpFmx2zU18rcvqSHohwEHIyIAIIwjUxc3Q7WV37Sge3K6jkLjeX2nTof+fZ10l+OyAokDAggA2gQARzBEAiBi63pVYQenxz9FrEq1od3fb3B1+xJ1lpp/OD7/94S8sgIgDAXbt0cNvy8IVX3TVscyXB7TCRPpls04QJRdsSIo2l8BRzBEAiBl9FulmYtZon/+GnvtAWrx8fkNVLOqj3RQql9WolEDvQIgf3JHA60e25ZoCyhLVtT/y4j3+3Weq74IqjDym4UTg9IBR1IhAwidwQx6xttU+RMpr2FzM9s4jOrQwjH3IzedG5kDCwLcIQI63ZBPPW3PWd25BrDe4jUpt/+57VDl6GFRkmhgIh8Oc1KuACICA6mkw39ZltOqJdusa1cK8GUDlEkpQkYLNUdT7Z7spYdxENkMak8AAACAAAAAgAQAAIAAIgICf2OZdX0u/1WhNq0CxoSxg4tlVuXxtrNCgqlLa1AFEJYQ2QxqTwAAAIAAAACABQAAgAA=",
    "psbt format error."
  },
  {
    "70736274ff01009a020000000258e87a21b56daf0c23be8e7070456c336f7cbaa5c8757924f545887bb2abdd750000000000ffffffff838d0427d0ec650a68aa46bb0b098aea4422c071b2ca78352a077959d07cea1d0100000000ffffffff0270aaf00800000000160014d85c2b71d0060b09c9886aeb815e50991dda124d00e1f5050000000016001400aea9a2e5f0f876a588df5546e8742d1d87008f00000000000100bb0200000001aad73931018bd25f84ae400b68848be09db706eac2ac18298babee71ab656f8b0000000048473044022058f6fc7c6a33e1b31548d481c826c015bd30135aad42cd67790dab66d2ad243b02204a1ced2604c6735b6393e5b41691dd78b00f0c5942fb9f751856faa938157dba01feffffff0280f0fa020000000017a9140fb9463421696b82c833af241c78c17ddbde493487d0f20a270100000017a91429ca74f8a08f81999428185c97b5d852e4063f6187650000000107da00473044022074018ad4180097b873323c0015720b3684cc8123891048e7dbcd9b55ad679c99022073d369b740e3eb53dcefa33823c8070514ca55a7dd9544f157c167913261118c01483045022100f61038b308dc1da865a34852746f015772934208c6d24454393cd99bdf2217770220056e675a675a6d0a02b85b14e5e29074d8a25a9b5760bea2816f661910a006ea01475221029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f2102dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d752ae0001012000c2eb0b0000000017a914b7f5faf40e3d40a5a459b1db3535f2b72fa921e8870107232200208c2353173743b595dfb4a07b72ba8e42e3797da74e87fe7d9d7497e3b20289030108da0400473044022062eb7a556107a7c73f45ac4ab5a1dddf6f7075fb1275969a7f383efff784bcb202200c05dbb7470dbf2f08557dd356c7325c1ed30913e996cd3840945db12228da5f01473044022065f45ba5998b59a27ffe1a7bed016af1f1f90d54b3aa8f7450aa5f56a25103bd02207f724703ad1edb96680b284b56d4ffcb88f7fb759eabbe08aa30f29b851383d20147522103089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc21023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7352ae00210203a9a4c37f5996d3aa25dbac6b570af0650394492942460b354753ed9eeca58710d90c6a4f000000800000008004000080002202027f6399757d2eff55a136ad02c684b1838b6556e5f1b6b34282a94b6b5005109610d90c6a4f00000080000000800500008000",
    "cHNidP8BAJoCAAAAAljoeiG1ba8MI76OcHBFbDNvfLqlyHV5JPVFiHuyq911AAAAAAD/////g40EJ9DsZQpoqka7CwmK6kQiwHGyyng1Kgd5WdB86h0BAAAAAP////8CcKrwCAAAAAAWABTYXCtx0AYLCcmIauuBXlCZHdoSTQDh9QUAAAAAFgAUAK6pouXw+HaliN9VRuh0LR2HAI8AAAAAAAEAuwIAAAABqtc5MQGL0l+ErkALaISL4J23BurCrBgpi6vucatlb4sAAAAASEcwRAIgWPb8fGoz4bMVSNSByCbAFb0wE1qtQs1neQ2rZtKtJDsCIEoc7SYExnNbY5PltBaR3XiwDwxZQvufdRhW+qk4FX26Af7///8CgPD6AgAAAAAXqRQPuUY0IWlrgsgzryQceMF9295JNIfQ8gonAQAAABepFCnKdPigj4GZlCgYXJe12FLkBj9hh2UAAAABB9oARzBEAiB0AYrUGACXuHMyPAAVcgs2hMyBI4kQSOfbzZtVrWecmQIgc9Npt0Dj61Pc76M4I8gHBRTKVafdlUTxV8FnkTJhEYwBSDBFAiEA9hA4swjcHahlo0hSdG8BV3KTQgjG0kRUOTzZm98iF3cCIAVuZ1pnWm0KArhbFOXikHTYolqbV2C+ooFvZhkQoAbqAUdSIQKVg785rgpgl0etGZrd1jT6YQhVnWxc05tMIYPxq5bgfyEC2rYf9JoU22p9ArDNH7t4/EsYMStbTlTa5Nui+/71NtdSrgABASAAwusLAAAAABepFLf1+vQOPUClpFmx2zU18rcvqSHohwEHIyIAIIwjUxc3Q7WV37Sge3K6jkLjeX2nTof+fZ10l+OyAokDAQjaBABHMEQCIGLrelVhB6fHP0WsSrWh3d9vcHX7EnWWmn84Pv/3hLyyAiAMBdu3Rw2/LwhVfdNWxzJcHtMJE+mWzThAlF2xIijaXwFHMEQCIGX0W6WZi1mif/4ae+0BavHx+Q1Us6qPdFCqX1aiUQO9AiB/ckcDrR7blmgLKEtW1P/LiPf7dZ6rvgiqMPKbhROD0gFHUiEDCJ3BDHrG21T5EymvYXMz2ziM6tDCMfcjN50bmQMLAtwhAjrdkE89bc9Z3bkGsN7iNSm3/7ntUOXoYVGSaGAiHw5zUq4AIQIDqaTDf1mW06ol26xrVwrwZQOUSSlCRgs1R1PtnuylhxDZDGpPAAAAgAAAAIAEAACAACICAn9jmXV9Lv9VoTatAsaEsYOLZVbl8bazQoKpS2tQBRCWENkMak8AAACAAAAAgAUAAIAA",
    "psbt format error."
  },
  {
    "70736274ff0100730200000001301ae986e516a1ec8ac5b4bc6573d32f83b465e23ad76167d68b38e730b4dbdb0000000000ffffffff02747b01000000000017a91403aa17ae882b5d0d54b25d63104e4ffece7b9ea2876043993b0000000017a914b921b1ba6f722e4bfa83b6557a3139986a42ec8387000000000001011f00ca9a3b00000000160014d2d94b64ae08587eefc8eeb187c601e939f9037c0203000100000000010016001462e9e982fff34dd8239610316b090cd2a3b747cb000100220020876bad832f1d168015ed41232a9ea65a1815d9ef13c0ef8759f64b5b2b278a65010125512103b7ce23a01c5b4bf00a642537cdfabb315b668332867478ef51309d2bd57f8a8751ae00",
    "cHNidP8BAHMCAAAAATAa6YblFqHsisW0vGVz0y+DtGXiOtdhZ9aLOOcwtNvbAAAAAAD/////AnR7AQAAAAAAF6kUA6oXrogrXQ1Usl1jEE5P/s57nqKHYEOZOwAAAAAXqRS5IbG6b3IuS/qDtlV6MTmYakLsg4cAAAAAAAEBHwDKmjsAAAAAFgAU0tlLZK4IWH7vyO6xh8YB6Tn5A3wCAwABAAAAAAEAFgAUYunpgv/zTdgjlhAxawkM0qO3R8sAAQAiACCHa62DLx0WgBXtQSMqnqZaGBXZ7xPA74dZ9ktbKyeKZQEBJVEhA7fOI6AcW0vwCmQlN836uzFbZoMyhnR471EwnSvVf4qHUa4A",
    "psbt format error."
  },
  {
    "70736274ff0100730200000001301ae986e516a1ec8ac5b4bc6573d32f83b465e23ad76167d68b38e730b4dbdb0000000000ffffffff02747b01000000000017a91403aa17ae882b5d0d54b25d63104e4ffece7b9ea2876043993b0000000017a914b921b1ba6f722e4bfa83b6557a3139986a42ec8387000000000001011f00ca9a3b00000000160014d2d94b64ae08587eefc8eeb187c601e939f9037c0002000016001462e9e982fff34dd8239610316b090cd2a3b747cb000100220020876bad832f1d168015ed41232a9ea65a1815d9ef13c0ef8759f64b5b2b278a65010125512103b7ce23a01c5b4bf00a642537cdfabb315b668332867478ef51309d2bd57f8a8751ae00",
    "cHNidP8BAHMCAAAAATAa6YblFqHsisW0vGVz0y+DtGXiOtdhZ9aLOOcwtNvbAAAAAAD/////AnR7AQAAAAAAF6kUA6oXrogrXQ1Usl1jEE5P/s57nqKHYEOZOwAAAAAXqRS5IbG6b3IuS/qDtlV6MTmYakLsg4cAAAAAAAEBHwDKmjsAAAAAFgAU0tlLZK4IWH7vyO6xh8YB6Tn5A3wAAgAAFgAUYunpgv/zTdgjlhAxawkM0qO3R8sAAQAiACCHa62DLx0WgBXtQSMqnqZaGBXZ7xPA74dZ9ktbKyeKZQEBJVEhA7fOI6AcW0vwCmQlN836uzFbZoMyhnR471EwnSvVf4qHUa4A",
    "psbt format error."
  },
  {
    "70736274ff0100730200000001301ae986e516a1ec8ac5b4bc6573d32f83b465e23ad76167d68b38e730b4dbdb0000000000ffffffff02747b01000000000017a91403aa17ae882b5d0d54b25d63104e4ffece7b9ea2876043993b0000000017a914b921b1ba6f722e4bfa83b6557a3139986a42ec8387000000000001011f00ca9a3b00000000160014d2d94b64ae08587eefc8eeb187c601e939f9037c00010016001462e9e982fff34dd8239610316b090cd2a3b747cb000100220020876bad832f1d168015ed41232a9ea65a1815d9ef13c0ef8759f64b5b2b278a6521010025512103b7ce23a01c5b4bf00a642537cdfabb315b668332867478ef51309d06d57f8a8751ae00",
    "cHNidP8BAHMCAAAAATAa6YblFqHsisW0vGVz0y+DtGXiOtdhZ9aLOOcwtNvbAAAAAAD/////AnR7AQAAAAAAF6kUA6oXrogrXQ1Usl1jEE5P/s57nqKHYEOZOwAAAAAXqRS5IbG6b3IuS/qDtlV6MTmYakLsg4cAAAAAAAEBHwDKmjsAAAAAFgAU0tlLZK4IWH7vyO6xh8YB6Tn5A3wAAQAWABRi6emC//NN2COWEDFrCQzSo7dHywABACIAIIdrrYMvHRaAFe1BIyqeploYFdnvE8Dvh1n2S1srJ4plIQEAJVEhA7fOI6AcW0vwCmQlN836uzFbZoMyhnR471EwnQbVf4qHUa4A",
    "psbt format error."
  },
  {
    "70736274ff0100750200000001268171371edff285e937adeea4b37b78000c0566cbb3ad64641713ca42171bf60000000000feffffff02d3dff505000000001976a914d0c59903c5bac2868760e90fd521a4665aa7652088ac00e1f5050000000017a9143545e6e33b832c47050f24d3eeb93c9c03948bc787b32e1300000100fda5010100000000010289a3c71eab4d20e0371bbba4cc698fa295c9463afa2e397f8533ccb62f9567e50100000017160014be18d152a9b012039daf3da7de4f53349eecb985ffffffff86f8aa43a71dff1448893a530a7237ef6b4608bbb2dd2d0171e63aec6a4890b40100000017160014fe3e9ef1a745e974d902c4355943abcb34bd5353ffffffff0200c2eb0b000000001976a91485cff1097fd9e008bb34af709c62197b38978a4888ac72fef84e2c00000017a914339725ba21efd62ac753a9bcd067d6c7a6a39d05870247304402202712be22e0270f394f568311dc7ca9a68970b8025fdd3b240229f07f8a5f3a240220018b38d7dcd314e734c9276bd6fb40f673325bc4baa144c800d2f2f02db2765c012103d2e15674941bad4a996372cb87e1856d3652606d98562fe39c5e9e7e413f210502483045022100d12b852d85dcd961d2f5f4ab660654df6eedcc794c0c33ce5cc309ffb5fce58d022067338a8e0e1725c197fb1a88af59f51e44e4255b20167c8684031c05d1f2592a01210223b72beef0965d10be0778efecd61fcac6f79a4ea169393380734464f84f2ab300000000000000",
    "cHNidP8BAHUCAAAAASaBcTce3/KF6Tet7qSze3gADAVmy7OtZGQXE8pCFxv2AAAAAAD+////AtPf9QUAAAAAGXapFNDFmQPFusKGh2DpD9UhpGZap2UgiKwA4fUFAAAAABepFDVF5uM7gyxHBQ8k0+65PJwDlIvHh7MuEwAAAQD9pQEBAAAAAAECiaPHHqtNIOA3G7ukzGmPopXJRjr6Ljl/hTPMti+VZ+UBAAAAFxYAFL4Y0VKpsBIDna89p95PUzSe7LmF/////4b4qkOnHf8USIk6UwpyN+9rRgi7st0tAXHmOuxqSJC0AQAAABcWABT+Pp7xp0XpdNkCxDVZQ6vLNL1TU/////8CAMLrCwAAAAAZdqkUhc/xCX/Z4Ai7NK9wnGIZeziXikiIrHL++E4sAAAAF6kUM5cluiHv1irHU6m80GfWx6ajnQWHAkcwRAIgJxK+IuAnDzlPVoMR3HyppolwuAJf3TskAinwf4pfOiQCIAGLONfc0xTnNMkna9b7QPZzMlvEuqFEyADS8vAtsnZcASED0uFWdJQbrUqZY3LLh+GFbTZSYG2YVi/jnF6efkE/IQUCSDBFAiEA0SuFLYXc2WHS9fSrZgZU327tzHlMDDPOXMMJ/7X85Y0CIGczio4OFyXBl/saiK9Z9R5E5CVbIBZ8hoQDHAXR8lkqASECI7cr7vCWXRC+B3jv7NYfysb3mk6haTkzgHNEZPhPKrMAAAAAAAAA",
    ""
  },
  {
    "70736274ff0100a00200000002ab0949a08c5af7c49b8212f417e2f15ab3f5c33dcf153821a8139f877a5b7be40000000000feffffffab0949a08c5af7c49b8212f417e2f15ab3f5c33dcf153821a8139f877a5b7be40100000000feffffff02603bea0b000000001976a914768a40bbd740cbe81d988e71de2a4d5c71396b1d88ac8e240000000000001976a9146f4620b553fa095e721b9ee0efe9fa039cca459788ac000000000001076a47304402204759661797c01b036b25928948686218347d89864b719e1f7fcf57d1e511658702205309eabf56aa4d8891ffd111fdf1336f3a29da866d7f8486d75546ceedaf93190121035cdc61fc7ba971c0b501a646a2a83b102cb43881217ca682dc86e2d73fa882920001012000e1f5050000000017a9143545e6e33b832c47050f24d3eeb93c9c03948bc787010416001485d13537f2e265405a34dbafa9e3dda01fb82308000000",
    "cHNidP8BAKACAAAAAqsJSaCMWvfEm4IS9Bfi8Vqz9cM9zxU4IagTn4d6W3vkAAAAAAD+////qwlJoIxa98SbghL0F+LxWrP1wz3PFTghqBOfh3pbe+QBAAAAAP7///8CYDvqCwAAAAAZdqkUdopAu9dAy+gdmI5x3ipNXHE5ax2IrI4kAAAAAAAAGXapFG9GILVT+glechue4O/p+gOcykWXiKwAAAAAAAEHakcwRAIgR1lmF5fAGwNrJZKJSGhiGDR9iYZLcZ4ff89X0eURZYcCIFMJ6r9Wqk2Ikf/REf3xM286KdqGbX+EhtdVRs7tr5MZASEDXNxh/HupccC1AaZGoqg7ECy0OIEhfKaC3Ibi1z+ogpIAAQEgAOH1BQAAAAAXqRQ1RebjO4MsRwUPJNPuuTycA5SLx4cBBBYAFIXRNTfy4mVAWjTbr6nj3aAfuCMIAAAA",
    ""
  },
  {
    "70736274ff0100750200000001268171371edff285e937adeea4b37b78000c0566cbb3ad64641713ca42171bf60000000000feffffff02d3dff505000000001976a914d0c59903c5bac2868760e90fd521a4665aa7652088ac00e1f5050000000017a9143545e6e33b832c47050f24d3eeb93c9c03948bc787b32e1300000100fda5010100000000010289a3c71eab4d20e0371bbba4cc698fa295c9463afa2e397f8533ccb62f9567e50100000017160014be18d152a9b012039daf3da7de4f53349eecb985ffffffff86f8aa43a71dff1448893a530a7237ef6b4608bbb2dd2d0171e63aec6a4890b40100000017160014fe3e9ef1a745e974d902c4355943abcb34bd5353ffffffff0200c2eb0b000000001976a91485cff1097fd9e008bb34af709c62197b38978a4888ac72fef84e2c00000017a914339725ba21efd62ac753a9bcd067d6c7a6a39d05870247304402202712be22e0270f394f568311dc7ca9a68970b8025fdd3b240229f07f8a5f3a240220018b38d7dcd314e734c9276bd6fb40f673325bc4baa144c800d2f2f02db2765c012103d2e15674941bad4a996372cb87e1856d3652606d98562fe39c5e9e7e413f210502483045022100d12b852d85dcd961d2f5f4ab660654df6eedcc794c0c33ce5cc309ffb5fce58d022067338a8e0e1725c197fb1a88af59f51e44e4255b20167c8684031c05d1f2592a01210223b72beef0965d10be0778efecd61fcac6f79a4ea169393380734464f84f2ab30000000001030401000000000000",
    "cHNidP8BAHUCAAAAASaBcTce3/KF6Tet7qSze3gADAVmy7OtZGQXE8pCFxv2AAAAAAD+////AtPf9QUAAAAAGXapFNDFmQPFusKGh2DpD9UhpGZap2UgiKwA4fUFAAAAABepFDVF5uM7gyxHBQ8k0+65PJwDlIvHh7MuEwAAAQD9pQEBAAAAAAECiaPHHqtNIOA3G7ukzGmPopXJRjr6Ljl/hTPMti+VZ+UBAAAAFxYAFL4Y0VKpsBIDna89p95PUzSe7LmF/////4b4qkOnHf8USIk6UwpyN+9rRgi7st0tAXHmOuxqSJC0AQAAABcWABT+Pp7xp0XpdNkCxDVZQ6vLNL1TU/////8CAMLrCwAAAAAZdqkUhc/xCX/Z4Ai7NK9wnGIZeziXikiIrHL++E4sAAAAF6kUM5cluiHv1irHU6m80GfWx6ajnQWHAkcwRAIgJxK+IuAnDzlPVoMR3HyppolwuAJf3TskAinwf4pfOiQCIAGLONfc0xTnNMkna9b7QPZzMlvEuqFEyADS8vAtsnZcASED0uFWdJQbrUqZY3LLh+GFbTZSYG2YVi/jnF6efkE/IQUCSDBFAiEA0SuFLYXc2WHS9fSrZgZU327tzHlMDDPOXMMJ/7X85Y0CIGczio4OFyXBl/saiK9Z9R5E5CVbIBZ8hoQDHAXR8lkqASECI7cr7vCWXRC+B3jv7NYfysb3mk6haTkzgHNEZPhPKrMAAAAAAQMEAQAAAAAAAA==",
    ""
  },
  {
    "70736274ff0100a00200000002ab0949a08c5af7c49b8212f417e2f15ab3f5c33dcf153821a8139f877a5b7be40000000000feffffffab0949a08c5af7c49b8212f417e2f15ab3f5c33dcf153821a8139f877a5b7be40100000000feffffff02603bea0b000000001976a914768a40bbd740cbe81d988e71de2a4d5c71396b1d88ac8e240000000000001976a9146f4620b553fa095e721b9ee0efe9fa039cca459788ac00000000000100df0200000001268171371edff285e937adeea4b37b78000c0566cbb3ad64641713ca42171bf6000000006a473044022070b2245123e6bf474d60c5b50c043d4c691a5d2435f09a34a7662a9dc251790a022001329ca9dacf280bdf30740ec0390422422c81cb45839457aeb76fc12edd95b3012102657d118d3357b8e0f4c2cd46db7b39f6d9c38d9a70abcb9b2de5dc8dbfe4ce31feffffff02d3dff505000000001976a914d0c59903c5bac2868760e90fd521a4665aa7652088ac00e1f5050000000017a9143545e6e33b832c47050f24d3eeb93c9c03948bc787b32e13000001012000e1f5050000000017a9143545e6e33b832c47050f24d3eeb93c9c03948bc787010416001485d13537f2e265405a34dbafa9e3dda01fb8230800220202ead596687ca806043edc3de116cdf29d5e9257c196cd055cf698c8d02bf24e9910b4a6ba670000008000000080020000800022020394f62be9df19952c5587768aeb7698061ad2c4a25c894f47d8c162b4d7213d0510b4a6ba6700000080010000800200008000",
    "cHNidP8BAKACAAAAAqsJSaCMWvfEm4IS9Bfi8Vqz9cM9zxU4IagTn4d6W3vkAAAAAAD+////qwlJoIxa98SbghL0F+LxWrP1wz3PFTghqBOfh3pbe+QBAAAAAP7///8CYDvqCwAAAAAZdqkUdopAu9dAy+gdmI5x3ipNXHE5ax2IrI4kAAAAAAAAGXapFG9GILVT+glechue4O/p+gOcykWXiKwAAAAAAAEA3wIAAAABJoFxNx7f8oXpN63upLN7eAAMBWbLs61kZBcTykIXG/YAAAAAakcwRAIgcLIkUSPmv0dNYMW1DAQ9TGkaXSQ18Jo0p2YqncJReQoCIAEynKnazygL3zB0DsA5BCJCLIHLRYOUV663b8Eu3ZWzASECZX0RjTNXuOD0ws1G23s59tnDjZpwq8ubLeXcjb/kzjH+////AtPf9QUAAAAAGXapFNDFmQPFusKGh2DpD9UhpGZap2UgiKwA4fUFAAAAABepFDVF5uM7gyxHBQ8k0+65PJwDlIvHh7MuEwAAAQEgAOH1BQAAAAAXqRQ1RebjO4MsRwUPJNPuuTycA5SLx4cBBBYAFIXRNTfy4mVAWjTbr6nj3aAfuCMIACICAurVlmh8qAYEPtw94RbN8p1eklfBls0FXPaYyNAr8k6ZELSmumcAAACAAAAAgAIAAIAAIgIDlPYr6d8ZlSxVh3aK63aYBhrSxKJciU9H2MFitNchPQUQtKa6ZwAAAIABAACAAgAAgAA=",
    ""
  },
  {
    "70736274ff0100550200000001279a2323a5dfb51fc45f220fa58b0fc13e1e3342792a85d7e36cd6333b5cbc390000000000ffffffff01a05aea0b000000001976a914ffe9c0061097cc3b636f2cb0460fa4fc427d2b4588ac0000000000010120955eea0b0000000017a9146345200f68d189e1adc0df1c4d16ea8f14c0dbeb87220203b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd4646304302200424b58effaaa694e1559ea5c93bbfd4a89064224055cdf070b6771469442d07021f5c8eb0fea6516d60b8acb33ad64ede60e8785bfb3aa94b99bdf86151db9a9a010104220020771fd18ad459666dd49f3d564e3dbc42f4c84774e360ada16816a8ed488d5681010547522103b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd462103de55d1e1dac805e3f8a58c1fbf9b94c02f3dbaafe127fefca4995f26f82083bd52ae220603b1341ccba7683b6af4f1238cd6e97e7167d569fac47f1e48d47541844355bd4610b4a6ba67000000800000008004000080220603de55d1e1dac805e3f8a58c1fbf9b94c02f3dbaafe127fefca4995f26f82083bd10b4a6ba670000008000000080050000800000",
    "cHNidP8BAFUCAAAAASeaIyOl37UfxF8iD6WLD8E+HjNCeSqF1+Ns1jM7XLw5AAAAAAD/////AaBa6gsAAAAAGXapFP/pwAYQl8w7Y28ssEYPpPxCfStFiKwAAAAAAAEBIJVe6gsAAAAAF6kUY0UgD2jRieGtwN8cTRbqjxTA2+uHIgIDsTQcy6doO2r08SOM1ul+cWfVafrEfx5I1HVBhENVvUZGMEMCIAQktY7/qqaU4VWepck7v9SokGQiQFXN8HC2dxRpRC0HAh9cjrD+plFtYLisszrWTt5g6Hhb+zqpS5m9+GFR25qaAQEEIgAgdx/RitRZZm3Unz1WTj28QvTIR3TjYK2haBao7UiNVoEBBUdSIQOxNBzLp2g7avTxI4zW6X5xZ9Vp+sR/HkjUdUGEQ1W9RiED3lXR4drIBeP4pYwfv5uUwC89uq/hJ/78pJlfJvggg71SriIGA7E0HMunaDtq9PEjjNbpfnFn1Wn6xH8eSNR1QYRDVb1GELSmumcAAACAAAAAgAQAAIAiBgPeVdHh2sgF4/iljB+/m5TALz26r+En/vykmV8m+CCDvRC0prpnAAAAgAAAAIAFAACAAAA=",
    ""
  },
  {
    "70736274ff01005202000000019dfc6628c26c5899fe1bd3dc338665bfd55d7ada10f6220973df2d386dec12760100000000ffffffff01f03dcd1d000000001600147b3a00bfdc14d27795c2b74901d09da6ef133579000000004f01043587cf02da3fd0088000000097048b1ad0445b1ec8275517727c87b4e4ebc18a203ffa0f94c01566bd38e9000351b743887ee1d40dc32a6043724f2d6459b3b5a4d73daec8fbae0472f3bc43e20cd90c6a4fae000080000000804f01043587cf02da3fd00880000001b90452427139cd78c2cff2444be353cd58605e3e513285e528b407fae3f6173503d30a5e97c8adbc557dac2ad9a7e39c1722ebac69e668b6f2667cc1d671c83cab0cd90c6a4fae000080010000800001012b0065cd1d000000002200202c5486126c4978079a814e13715d65f36459e4d6ccaded266d0508645bafa6320105475221029da12cdb5b235692b91536afefe5c91c3ab9473d8e43b533836ab456299c88712103372b34234ed7cf9c1fea5d05d441557927be9542b162eb02e1ab2ce80224c00b52ae2206029da12cdb5b235692b91536afefe5c91c3ab9473d8e43b533836ab456299c887110d90c6a4fae0000800000008000000000220603372b34234ed7cf9c1fea5d05d441557927be9542b162eb02e1ab2ce80224c00b10d90c6a4fae0000800100008000000000002202039eff1f547a1d5f92dfa2ba7af6ac971a4bd03ba4a734b03156a256b8ad3a1ef910ede45cc500000080000000800100008000",
    "cHNidP8BAFICAAAAAZ38ZijCbFiZ/hvT3DOGZb/VXXraEPYiCXPfLTht7BJ2AQAAAAD/////AfA9zR0AAAAAFgAUezoAv9wU0neVwrdJAdCdpu8TNXkAAAAATwEENYfPAto/0AiAAAAAlwSLGtBEWx7IJ1UXcnyHtOTrwYogP/oPlMAVZr046QADUbdDiH7h1A3DKmBDck8tZFmztaTXPa7I+64EcvO8Q+IM2QxqT64AAIAAAACATwEENYfPAto/0AiAAAABuQRSQnE5zXjCz/JES+NTzVhgXj5RMoXlKLQH+uP2FzUD0wpel8itvFV9rCrZp+OcFyLrrGnmaLbyZnzB1nHIPKsM2QxqT64AAIABAACAAAEBKwBlzR0AAAAAIgAgLFSGEmxJeAeagU4TcV1l82RZ5NbMre0mbQUIZFuvpjIBBUdSIQKdoSzbWyNWkrkVNq/v5ckcOrlHPY5DtTODarRWKZyIcSEDNys0I07Xz5wf6l0F1EFVeSe+lUKxYusC4ass6AIkwAtSriIGAp2hLNtbI1aSuRU2r+/lyRw6uUc9jkO1M4NqtFYpnIhxENkMak+uAACAAAAAgAAAAAAiBgM3KzQjTtfPnB/qXQXUQVV5J76VQrFi6wLhqyzoAiTACxDZDGpPrgAAgAEAAIAAAAAAACICA57/H1R6HV+S36K6evaslxpL0DukpzSwMVaiVritOh75EO3kXMUAAACAAAAAgAEAAIAA",
    ""
  },
  {
    "70736274ff01003f0200000001ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff0000000000ffffffff010000000000000000036a010000000000000a0f0102030405060708090f0102030405060708090a0b0c0d0e0f0000",
    "cHNidP8BAD8CAAAAAf//////////////////////////////////////////AAAAAAD/////AQAAAAAAAAAAA2oBAAAAAAAACg8BAgMEBQYHCAkPAQIDBAUGBwgJCgsMDQ4PAAA=",
    ""
  },
  {
    "70736274ff01009d0100000002710ea76ab45c5cb6438e607e59cc037626981805ae9e0dfd9089012abb0be5350100000000ffffffff190994d6a8b3c8c82ccbcfb2fba4106aa06639b872a8d447465c0d42588d6d670000000000ffffffff0200e1f505000000001976a914b6bc2c0ee5655a843d79afedd0ccc3f7dd64340988ac605af405000000001600141188ef8e4ce0449eaac8fb141cbf5a1176e6a088000000004f010488b21e039e530cac800000003dbc8a5c9769f031b17e77fea1518603221a18fd18f2b9a54c6c8c1ac75cbc3502f230584b155d1c7f1cd45120a653c48d650b431b67c5b2c13f27d7142037c1691027569c503100008000000080000000800001011f00e1f5050000000016001433b982f91b28f160c920b4ab95e58ce50dda3a4a220203309680f33c7de38ea6a47cd4ecd66f1f5a49747c6ffb8808ed09039243e3ad5c47304402202d704ced830c56a909344bd742b6852dccd103e963bae92d38e75254d2bb424502202d86c437195df46c0ceda084f2a291c3da2d64070f76bf9b90b195e7ef28f77201220603309680f33c7de38ea6a47cd4ecd66f1f5a49747c6ffb8808ed09039243e3ad5c1827569c5031000080000000800000008000000000010000000001011f00e1f50500000000160014388fb944307eb77ef45197d0b0b245e079f011de220202c777161f73d0b7c72b9ee7bde650293d13f095bc7656ad1f525da5fd2e10b11047304402204cb1fb5f869c942e0e26100576125439179ae88dca8a9dc3ba08f7953988faa60220521f49ca791c27d70e273c9b14616985909361e25be274ea200d7e08827e514d01220602c777161f73d0b7c72b9ee7bde650293d13f095bc7656ad1f525da5fd2e10b1101827569c5031000080000000800000008000000000000000000000220202d20ca502ee289686d21815bd43a80637b0698e1fbcdbe4caed445f6c1a0a90ef1827569c50310000800000008000000080000000000400000000",
    "cHNidP8BAJ0BAAAAAnEOp2q0XFy2Q45gflnMA3YmmBgFrp4N/ZCJASq7C+U1AQAAAAD/////GQmU1qizyMgsy8+y+6QQaqBmObhyqNRHRlwNQliNbWcAAAAAAP////8CAOH1BQAAAAAZdqkUtrwsDuVlWoQ9ea/t0MzD991kNAmIrGBa9AUAAAAAFgAUEYjvjkzgRJ6qyPsUHL9aEXbmoIgAAAAATwEEiLIeA55TDKyAAAAAPbyKXJdp8DGxfnf+oVGGAyIaGP0Y8rmlTGyMGsdcvDUC8jBYSxVdHH8c1FEgplPEjWULQxtnxbLBPyfXFCA3wWkQJ1acUDEAAIAAAACAAAAAgAABAR8A4fUFAAAAABYAFDO5gvkbKPFgySC0q5XljOUN2jpKIgIDMJaA8zx9446mpHzU7NZvH1pJdHxv+4gI7QkDkkPjrVxHMEQCIC1wTO2DDFapCTRL10K2hS3M0QPpY7rpLTjnUlTSu0JFAiAthsQ3GV30bAztoITyopHD2i1kBw92v5uQsZXn7yj3cgEiBgMwloDzPH3jjqakfNTs1m8fWkl0fG/7iAjtCQOSQ+OtXBgnVpxQMQAAgAAAAIAAAACAAAAAAAEAAAAAAQEfAOH1BQAAAAAWABQ4j7lEMH63fvRRl9CwskXgefAR3iICAsd3Fh9z0LfHK57nveZQKT0T8JW8dlatH1Jdpf0uELEQRzBEAiBMsftfhpyULg4mEAV2ElQ5F5rojcqKncO6CPeVOYj6pgIgUh9JynkcJ9cOJzybFGFphZCTYeJb4nTqIA1+CIJ+UU0BIgYCx3cWH3PQt8crnue95lApPRPwlbx2Vq0fUl2l/S4QsRAYJ1acUDEAAIAAAACAAAAAgAAAAAAAAAAAAAAiAgLSDKUC7iiWhtIYFb1DqAY3sGmOH7zb5MrtRF9sGgqQ7xgnVpxQMQAAgAAAAIAAAACAAAAAAAQAAAAA",
    ""
  },
  {
    "70736274ff0100a00200000002ab0949a08c5af7c49b8212f417e2f15ab3f5c33dcf153821a8139f877a5b7be40000000000feffffffab0949a08c5af7c49b8212f417e2f15ab3f5c33dcf153821a8139f877a5b7be40100000000feffffff02603bea0b000000001976a914768a40bbd740cbe81d988e71de2a4d5c71396b1d88ac8e240000000000001976a9146f4620b553fa095e721b9ee0efe9fa039cca459788ac0000000000010122d3dff505000000001976a914d48ed3110b94014cb114bd32d6f4d066dc74256b88ac0001012000e1f5050000000017a9143545e6e33b832c47050f24d3eeb93c9c03948bc787010416001485d13537f2e265405a34dbafa9e3dda01fb8230800220202ead596687ca806043edc3de116cdf29d5e9257c196cd055cf698c8d02bf24e9910b4a6ba670000008000000080020000800022020394f62be9df19952c5587768aeb7698061ad2c4a25c894f47d8c162b4d7213d0510b4a6ba6700000080010000800200008000",
    "cHNidP8BAKACAAAAAqsJSaCMWvfEm4IS9Bfi8Vqz9cM9zxU4IagTn4d6W3vkAAAAAAD+////qwlJoIxa98SbghL0F+LxWrP1wz3PFTghqBOfh3pbe+QBAAAAAP7///8CYDvqCwAAAAAZdqkUdopAu9dAy+gdmI5x3ipNXHE5ax2IrI4kAAAAAAAAGXapFG9GILVT+glechue4O/p+gOcykWXiKwAAAAAAAEBItPf9QUAAAAAGXapFNSO0xELlAFMsRS9Mtb00GbcdCVriKwAAQEgAOH1BQAAAAAXqRQ1RebjO4MsRwUPJNPuuTycA5SLx4cBBBYAFIXRNTfy4mVAWjTbr6nj3aAfuCMIACICAurVlmh8qAYEPtw94RbN8p1eklfBls0FXPaYyNAr8k6ZELSmumcAAACAAAAAgAIAAIAAIgIDlPYr6d8ZlSxVh3aK63aYBhrSxKJciU9H2MFitNchPQUQtKa6ZwAAAIABAACAAgAAgAA=",
    ""
  },
  {
    "70736274ff01009a020000000258e87a21b56daf0c23be8e7070456c336f7cbaa5c8757924f545887bb2abdd750000000000ffffffff838d0427d0ec650a68aa46bb0b098aea4422c071b2ca78352a077959d07cea1d0100000000ffffffff0270aaf00800000000160014d85c2b71d0060b09c9886aeb815e50991dda124d00e1f5050000000016001400aea9a2e5f0f876a588df5546e8742d1d87008f00000000000100bb0200000001aad73931018bd25f84ae400b68848be09db706eac2ac18298babee71ab656f8b0000000048473044022058f6fc7c6a33e1b31548d481c826c015bd30135aad42cd67790dab66d2ad243b02204a1ced2604c6735b6393e5b41691dd78b00f0c5942fb9f751856faa938157dba01feffffff0280f0fa020000000017a9140fb9463421696b82c833af241c78c17ddbde493487d0f20a270100000017a91429ca74f8a08f81999428185c97b5d852e4063f618765000000220202dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d7483045022100f61038b308dc1da865a34852746f015772934208c6d24454393cd99bdf2217770220056e675a675a6d0a02b85b14e5e29074d8a25a9b5760bea2816f661910a006ea01010304010000000104475221029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f2102dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d752af2206029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f10d90c6a4f000000800000008000000080220602dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d710d90c6a4f0000008000000080010000800001012000c2eb0b0000000017a914b7f5faf40e3d40a5a459b1db3535f2b72fa921e8872202023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e73473044022065f45ba5998b59a27ffe1a7bed016af1f1f90d54b3aa8f7450aa5f56a25103bd02207f724703ad1edb96680b284b56d4ffcb88f7fb759eabbe08aa30f29b851383d2010103040100000001042200208c2353173743b595dfb4a07b72ba8e42e3797da74e87fe7d9d7497e3b2028903010547522103089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc21023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7352ae2206023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7310d90c6a4f000000800000008003000080220603089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc10d90c6a4f00000080000000800200008000220203a9a4c37f5996d3aa25dbac6b570af0650394492942460b354753ed9eeca5877110d90c6a4f000000800000008004000080002202027f6399757d2eff55a136ad02c684b1838b6556e5f1b6b34282a94b6b5005109610d90c6a4f00000080000000800500008000",
    "cHNidP8BAJoCAAAAAljoeiG1ba8MI76OcHBFbDNvfLqlyHV5JPVFiHuyq911AAAAAAD/////g40EJ9DsZQpoqka7CwmK6kQiwHGyyng1Kgd5WdB86h0BAAAAAP////8CcKrwCAAAAAAWABTYXCtx0AYLCcmIauuBXlCZHdoSTQDh9QUAAAAAFgAUAK6pouXw+HaliN9VRuh0LR2HAI8AAAAAAAEAuwIAAAABqtc5MQGL0l+ErkALaISL4J23BurCrBgpi6vucatlb4sAAAAASEcwRAIgWPb8fGoz4bMVSNSByCbAFb0wE1qtQs1neQ2rZtKtJDsCIEoc7SYExnNbY5PltBaR3XiwDwxZQvufdRhW+qk4FX26Af7///8CgPD6AgAAAAAXqRQPuUY0IWlrgsgzryQceMF9295JNIfQ8gonAQAAABepFCnKdPigj4GZlCgYXJe12FLkBj9hh2UAAAAiAgLath/0mhTban0CsM0fu3j8SxgxK1tOVNrk26L7/vU210gwRQIhAPYQOLMI3B2oZaNIUnRvAVdyk0IIxtJEVDk82ZvfIhd3AiAFbmdaZ1ptCgK4WxTl4pB02KJam1dgvqKBb2YZEKAG6gEBAwQBAAAAAQRHUiEClYO/Oa4KYJdHrRma3dY0+mEIVZ1sXNObTCGD8auW4H8hAtq2H/SaFNtqfQKwzR+7ePxLGDErW05U2uTbovv+9TbXUq8iBgKVg785rgpgl0etGZrd1jT6YQhVnWxc05tMIYPxq5bgfxDZDGpPAAAAgAAAAIAAAACAIgYC2rYf9JoU22p9ArDNH7t4/EsYMStbTlTa5Nui+/71NtcQ2QxqTwAAAIAAAACAAQAAgAABASAAwusLAAAAABepFLf1+vQOPUClpFmx2zU18rcvqSHohyICAjrdkE89bc9Z3bkGsN7iNSm3/7ntUOXoYVGSaGAiHw5zRzBEAiBl9FulmYtZon/+GnvtAWrx8fkNVLOqj3RQql9WolEDvQIgf3JHA60e25ZoCyhLVtT/y4j3+3Weq74IqjDym4UTg9IBAQMEAQAAAAEEIgAgjCNTFzdDtZXftKB7crqOQuN5fadOh/59nXSX47ICiQMBBUdSIQMIncEMesbbVPkTKa9hczPbOIzq0MIx9yM3nRuZAwsC3CECOt2QTz1tz1nduQaw3uI1Kbf/ue1Q5ehhUZJoYCIfDnNSriIGAjrdkE89bc9Z3bkGsN7iNSm3/7ntUOXoYVGSaGAiHw5zENkMak8AAACAAAAAgAMAAIAiBgMIncEMesbbVPkTKa9hczPbOIzq0MIx9yM3nRuZAwsC3BDZDGpPAAAAgAAAAIACAACAACICA6mkw39ZltOqJdusa1cK8GUDlEkpQkYLNUdT7Z7spYdxENkMak8AAACAAAAAgAQAAIAAIgICf2OZdX0u/1WhNq0CxoSxg4tlVuXxtrNCgqlLa1AFEJYQ2QxqTwAAAIAAAACABQAAgAA=",
    ""
  },
  {
    "70736274ff01009a020000000258e87a21b56daf0c23be8e7070456c336f7cbaa5c8757924f545887bb2abdd750000000000ffffffff838d0427d0ec650a68aa46bb0b098aea4422c071b2ca78352a077959d07cea1d0100000000ffffffff0270aaf00800000000160014d85c2b71d0060b09c9886aeb815e50991dda124d00e1f5050000000016001400aea9a2e5f0f876a588df5546e8742d1d87008f00000000000100bb0200000001aad73931018bd25f84ae400b68848be09db706eac2ac18298babee71ab656f8b0000000048473044022058f6fc7c6a33e1b31548d481c826c015bd30135aad42cd67790dab66d2ad243b02204a1ced2604c6735b6393e5b41691dd78b00f0c5942fb9f751856faa938157dba01feffffff0280f0fa020000000017a9140fb9463421696b82c833af241c78c17ddbde493487d0f20a270100000017a91429ca74f8a08f81999428185c97b5d852e4063f618765000000220202dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d7483045022100f61038b308dc1da865a34852746f015772934208c6d24454393cd99bdf2217770220056e675a675a6d0a02b85b14e5e29074d8a25a9b5760bea2816f661910a006ea01010304010000000104475221029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f2102dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d752ae2206029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f10d90c6a4f000000800000008000000080220602dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d710d90c6a4f0000008000000080010000800001012000c2eb0b0000000017a914b7f5faf40e3d40a5a459b1db3535f2b72fa921e8872202023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e73473044022065f45ba5998b59a27ffe1a7bed016af1f1f90d54b3aa8f7450aa5f56a25103bd02207f724703ad1edb96680b284b56d4ffcb88f7fb759eabbe08aa30f29b851383d2010103040100000001042200208c2353173743b595dfb4a07b72ba8e42e3797da74e87fe7d9d7497e3b2028900010547522103089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc21023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7352ae2206023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7310d90c6a4f000000800000008003000080220603089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc10d90c6a4f00000080000000800200008000220203a9a4c37f5996d3aa25dbac6b570af0650394492942460b354753ed9eeca5877110d90c6a4f000000800000008004000080002202027f6399757d2eff55a136ad02c684b1838b6556e5f1b6b34282a94b6b5005109610d90c6a4f00000080000000800500008000",
    "cHNidP8BAJoCAAAAAljoeiG1ba8MI76OcHBFbDNvfLqlyHV5JPVFiHuyq911AAAAAAD/////g40EJ9DsZQpoqka7CwmK6kQiwHGyyng1Kgd5WdB86h0BAAAAAP////8CcKrwCAAAAAAWABTYXCtx0AYLCcmIauuBXlCZHdoSTQDh9QUAAAAAFgAUAK6pouXw+HaliN9VRuh0LR2HAI8AAAAAAAEAuwIAAAABqtc5MQGL0l+ErkALaISL4J23BurCrBgpi6vucatlb4sAAAAASEcwRAIgWPb8fGoz4bMVSNSByCbAFb0wE1qtQs1neQ2rZtKtJDsCIEoc7SYExnNbY5PltBaR3XiwDwxZQvufdRhW+qk4FX26Af7///8CgPD6AgAAAAAXqRQPuUY0IWlrgsgzryQceMF9295JNIfQ8gonAQAAABepFCnKdPigj4GZlCgYXJe12FLkBj9hh2UAAAAiAgLath/0mhTban0CsM0fu3j8SxgxK1tOVNrk26L7/vU210gwRQIhAPYQOLMI3B2oZaNIUnRvAVdyk0IIxtJEVDk82ZvfIhd3AiAFbmdaZ1ptCgK4WxTl4pB02KJam1dgvqKBb2YZEKAG6gEBAwQBAAAAAQRHUiEClYO/Oa4KYJdHrRma3dY0+mEIVZ1sXNObTCGD8auW4H8hAtq2H/SaFNtqfQKwzR+7ePxLGDErW05U2uTbovv+9TbXUq4iBgKVg785rgpgl0etGZrd1jT6YQhVnWxc05tMIYPxq5bgfxDZDGpPAAAAgAAAAIAAAACAIgYC2rYf9JoU22p9ArDNH7t4/EsYMStbTlTa5Nui+/71NtcQ2QxqTwAAAIAAAACAAQAAgAABASAAwusLAAAAABepFLf1+vQOPUClpFmx2zU18rcvqSHohyICAjrdkE89bc9Z3bkGsN7iNSm3/7ntUOXoYVGSaGAiHw5zRzBEAiBl9FulmYtZon/+GnvtAWrx8fkNVLOqj3RQql9WolEDvQIgf3JHA60e25ZoCyhLVtT/y4j3+3Weq74IqjDym4UTg9IBAQMEAQAAAAEEIgAgjCNTFzdDtZXftKB7crqOQuN5fadOh/59nXSX47ICiQABBUdSIQMIncEMesbbVPkTKa9hczPbOIzq0MIx9yM3nRuZAwsC3CECOt2QTz1tz1nduQaw3uI1Kbf/ue1Q5ehhUZJoYCIfDnNSriIGAjrdkE89bc9Z3bkGsN7iNSm3/7ntUOXoYVGSaGAiHw5zENkMak8AAACAAAAAgAMAAIAiBgMIncEMesbbVPkTKa9hczPbOIzq0MIx9yM3nRuZAwsC3BDZDGpPAAAAgAAAAIACAACAACICA6mkw39ZltOqJdusa1cK8GUDlEkpQkYLNUdT7Z7spYdxENkMak8AAACAAAAAgAQAAIAAIgICf2OZdX0u/1WhNq0CxoSxg4tlVuXxtrNCgqlLa1AFEJYQ2QxqTwAAAIAAAACABQAAgAA=",
    ""
  },
  {
    "70736274ff01009a020000000258e87a21b56daf0c23be8e7070456c336f7cbaa5c8757924f545887bb2abdd750000000000ffffffff838d0427d0ec650a68aa46bb0b098aea4422c071b2ca78352a077959d07cea1d0100000000ffffffff0270aaf00800000000160014d85c2b71d0060b09c9886aeb815e50991dda124d00e1f5050000000016001400aea9a2e5f0f876a588df5546e8742d1d87008f00000000000100bb0200000001aad73931018bd25f84ae400b68848be09db706eac2ac18298babee71ab656f8b0000000048473044022058f6fc7c6a33e1b31548d481c826c015bd30135aad42cd67790dab66d2ad243b02204a1ced2604c6735b6393e5b41691dd78b00f0c5942fb9f751856faa938157dba01feffffff0280f0fa020000000017a9140fb9463421696b82c833af241c78c17ddbde493487d0f20a270100000017a91429ca74f8a08f81999428185c97b5d852e4063f618765000000220202dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d7483045022100f61038b308dc1da865a34852746f015772934208c6d24454393cd99bdf2217770220056e675a675a6d0a02b85b14e5e29074d8a25a9b5760bea2816f661910a006ea01010304010000000104475221029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f2102dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d752ae2206029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f10d90c6a4f000000800000008000000080220602dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d710d90c6a4f0000008000000080010000800001012000c2eb0b0000000017a914b7f5faf40e3d40a5a459b1db3535f2b72fa921e8872202023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e73473044022065f45ba5998b59a27ffe1a7bed016af1f1f90d54b3aa8f7450aa5f56a25103bd02207f724703ad1edb96680b284b56d4ffcb88f7fb759eabbe08aa30f29b851383d2010103040100000001042200208c2353173743b595dfb4a07b72ba8e42e3797da74e87fe7d9d7497e3b2028903010547522103089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc21023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7352ad2206023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7310d90c6a4f000000800000008003000080220603089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc10d90c6a4f00000080000000800200008000220203a9a4c37f5996d3aa25dbac6b570af0650394492942460b354753ed9eeca5877110d90c6a4f000000800000008004000080002202027f6399757d2eff55a136ad02c684b1838b6556e5f1b6b34282a94b6b5005109610d90c6a4f00000080000000800500008000",
    "cHNidP8BAJoCAAAAAljoeiG1ba8MI76OcHBFbDNvfLqlyHV5JPVFiHuyq911AAAAAAD/////g40EJ9DsZQpoqka7CwmK6kQiwHGyyng1Kgd5WdB86h0BAAAAAP////8CcKrwCAAAAAAWABTYXCtx0AYLCcmIauuBXlCZHdoSTQDh9QUAAAAAFgAUAK6pouXw+HaliN9VRuh0LR2HAI8AAAAAAAEAuwIAAAABqtc5MQGL0l+ErkALaISL4J23BurCrBgpi6vucatlb4sAAAAASEcwRAIgWPb8fGoz4bMVSNSByCbAFb0wE1qtQs1neQ2rZtKtJDsCIEoc7SYExnNbY5PltBaR3XiwDwxZQvufdRhW+qk4FX26Af7///8CgPD6AgAAAAAXqRQPuUY0IWlrgsgzryQceMF9295JNIfQ8gonAQAAABepFCnKdPigj4GZlCgYXJe12FLkBj9hh2UAAAAiAgLath/0mhTban0CsM0fu3j8SxgxK1tOVNrk26L7/vU210gwRQIhAPYQOLMI3B2oZaNIUnRvAVdyk0IIxtJEVDk82ZvfIhd3AiAFbmdaZ1ptCgK4WxTl4pB02KJam1dgvqKBb2YZEKAG6gEBAwQBAAAAAQRHUiEClYO/Oa4KYJdHrRma3dY0+mEIVZ1sXNObTCGD8auW4H8hAtq2H/SaFNtqfQKwzR+7ePxLGDErW05U2uTbovv+9TbXUq4iBgKVg785rgpgl0etGZrd1jT6YQhVnWxc05tMIYPxq5bgfxDZDGpPAAAAgAAAAIAAAACAIgYC2rYf9JoU22p9ArDNH7t4/EsYMStbTlTa5Nui+/71NtcQ2QxqTwAAAIAAAACAAQAAgAABASAAwusLAAAAABepFLf1+vQOPUClpFmx2zU18rcvqSHohyICAjrdkE89bc9Z3bkGsN7iNSm3/7ntUOXoYVGSaGAiHw5zRzBEAiBl9FulmYtZon/+GnvtAWrx8fkNVLOqj3RQql9WolEDvQIgf3JHA60e25ZoCyhLVtT/y4j3+3Weq74IqjDym4UTg9IBAQMEAQAAAAEEIgAgjCNTFzdDtZXftKB7crqOQuN5fadOh/59nXSX47ICiQMBBUdSIQMIncEMesbbVPkTKa9hczPbOIzq0MIx9yM3nRuZAwsC3CECOt2QTz1tz1nduQaw3uI1Kbf/ue1Q5ehhUZJoYCIfDnNSrSIGAjrdkE89bc9Z3bkGsN7iNSm3/7ntUOXoYVGSaGAiHw5zENkMak8AAACAAAAAgAMAAIAiBgMIncEMesbbVPkTKa9hczPbOIzq0MIx9yM3nRuZAwsC3BDZDGpPAAAAgAAAAIACAACAACICA6mkw39ZltOqJdusa1cK8GUDlEkpQkYLNUdT7Z7spYdxENkMak8AAACAAAAAgAQAAIAAIgICf2OZdX0u/1WhNq0CxoSxg4tlVuXxtrNCgqlLa1AFEJYQ2QxqTwAAAIAAAACABQAAgAA=",
    ""
  },
  {
    "70736274ff01009a020000000258e87a21b56daf0c23be8e7070456c336f7cbaa5c8757924f545887bb2abdd750000000000ffffffff838d0427d0ec650a68aa46bb0b098aea4422c071b2ca78352a077959d07cea1d0100000000ffffffff0270aaf00800000000160014d85c2b71d0060b09c9886aeb815e50991dda124d00e1f5050000000016001400aea9a2e5f0f876a588df5546e8742d1d87008f00000000000100bb0200000001aad73931018bd25f84ae400b68848be09db706eac2ac18298babee71ab656f8b0000000048473044022058f6fc7c6a33e1b31548d481c826c015bd30135aad42cd67790dab66d2ad243b02204a1ced2604c6735b6393e5b41691dd78b00f0c5942fb9f751856faa938157dba01feffffff0280f0fa020000000017a9140fb9463421696b82c833af241c78c17ddbde493487d0f20a270100000017a91429ca74f8a08f81999428185c97b5d852e4063f6187650000000104475221029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f2102dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d752ae2206029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f10d90c6a4f000000800000008000000080220602dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d710d90c6a4f0000008000000080010000800001012000c2eb0b0000000017a914b7f5faf40e3d40a5a459b1db3535f2b72fa921e88701042200208c2353173743b595dfb4a07b72ba8e42e3797da74e87fe7d9d7497e3b2028903010547522103089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc21023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7352ae2206023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7310d90c6a4f000000800000008003000080220603089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc10d90c6a4f00000080000000800200008000220203a9a4c37f5996d3aa25dbac6b570af0650394492942460b354753ed9eeca5877110d90c6a4f000000800000008004000080002202027f6399757d2eff55a136ad02c684b1838b6556e5f1b6b34282a94b6b5005109610d90c6a4f00000080000000800500008000",
    "cHNidP8BAJoCAAAAAljoeiG1ba8MI76OcHBFbDNvfLqlyHV5JPVFiHuyq911AAAAAAD/////g40EJ9DsZQpoqka7CwmK6kQiwHGyyng1Kgd5WdB86h0BAAAAAP////8CcKrwCAAAAAAWABTYXCtx0AYLCcmIauuBXlCZHdoSTQDh9QUAAAAAFgAUAK6pouXw+HaliN9VRuh0LR2HAI8AAAAAAAEAuwIAAAABqtc5MQGL0l+ErkALaISL4J23BurCrBgpi6vucatlb4sAAAAASEcwRAIgWPb8fGoz4bMVSNSByCbAFb0wE1qtQs1neQ2rZtKtJDsCIEoc7SYExnNbY5PltBaR3XiwDwxZQvufdRhW+qk4FX26Af7///8CgPD6AgAAAAAXqRQPuUY0IWlrgsgzryQceMF9295JNIfQ8gonAQAAABepFCnKdPigj4GZlCgYXJe12FLkBj9hh2UAAAABBEdSIQKVg785rgpgl0etGZrd1jT6YQhVnWxc05tMIYPxq5bgfyEC2rYf9JoU22p9ArDNH7t4/EsYMStbTlTa5Nui+/71NtdSriIGApWDvzmuCmCXR60Zmt3WNPphCFWdbFzTm0whg/GrluB/ENkMak8AAACAAAAAgAAAAIAiBgLath/0mhTban0CsM0fu3j8SxgxK1tOVNrk26L7/vU21xDZDGpPAAAAgAAAAIABAACAAAEBIADC6wsAAAAAF6kUt/X69A49QKWkWbHbNTXyty+pIeiHAQQiACCMI1MXN0O1ld+0oHtyuo5C43l9p06H/n2ddJfjsgKJAwEFR1IhAwidwQx6xttU+RMpr2FzM9s4jOrQwjH3IzedG5kDCwLcIQI63ZBPPW3PWd25BrDe4jUpt/+57VDl6GFRkmhgIh8Oc1KuIgYCOt2QTz1tz1nduQaw3uI1Kbf/ue1Q5ehhUZJoYCIfDnMQ2QxqTwAAAIAAAACAAwAAgCIGAwidwQx6xttU+RMpr2FzM9s4jOrQwjH3IzedG5kDCwLcENkMak8AAACAAAAAgAIAAIAAIgIDqaTDf1mW06ol26xrVwrwZQOUSSlCRgs1R1Ptnuylh3EQ2QxqTwAAAIAAAACABAAAgAAiAgJ/Y5l1fS7/VaE2rQLGhLGDi2VW5fG2s0KCqUtrUAUQlhDZDGpPAAAAgAAAAIAFAACAAA==",
    ""
  },
  {
    "70736274ff01009a020000000258e87a21b56daf0c23be8e7070456c336f7cbaa5c8757924f545887bb2abdd750000000000ffffffff838d0427d0ec650a68aa46bb0b098aea4422c071b2ca78352a077959d07cea1d0100000000ffffffff0270aaf00800000000160014d85c2b71d0060b09c9886aeb815e50991dda124d00e1f5050000000016001400aea9a2e5f0f876a588df5546e8742d1d87008f00000000000100bb0200000001aad73931018bd25f84ae400b68848be09db706eac2ac18298babee71ab656f8b0000000048473044022058f6fc7c6a33e1b31548d481c826c015bd30135aad42cd67790dab66d2ad243b02204a1ced2604c6735b6393e5b41691dd78b00f0c5942fb9f751856faa938157dba01feffffff0280f0fa020000000017a9140fb9463421696b82c833af241c78c17ddbde493487d0f20a270100000017a91429ca74f8a08f81999428185c97b5d852e4063f618765000000010304010000000104475221029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f2102dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d752ae2206029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f10d90c6a4f000000800000008000000080220602dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d710d90c6a4f0000008000000080010000800001012000c2eb0b0000000017a914b7f5faf40e3d40a5a459b1db3535f2b72fa921e8870103040100000001042200208c2353173743b595dfb4a07b72ba8e42e3797da74e87fe7d9d7497e3b2028903010547522103089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc21023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7352ae2206023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7310d90c6a4f000000800000008003000080220603089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc10d90c6a4f00000080000000800200008000220203a9a4c37f5996d3aa25dbac6b570af0650394492942460b354753ed9eeca5877110d90c6a4f000000800000008004000080002202027f6399757d2eff55a136ad02c684b1838b6556e5f1b6b34282a94b6b5005109610d90c6a4f00000080000000800500008000",
    "cHNidP8BAJoCAAAAAljoeiG1ba8MI76OcHBFbDNvfLqlyHV5JPVFiHuyq911AAAAAAD/////g40EJ9DsZQpoqka7CwmK6kQiwHGyyng1Kgd5WdB86h0BAAAAAP////8CcKrwCAAAAAAWABTYXCtx0AYLCcmIauuBXlCZHdoSTQDh9QUAAAAAFgAUAK6pouXw+HaliN9VRuh0LR2HAI8AAAAAAAEAuwIAAAABqtc5MQGL0l+ErkALaISL4J23BurCrBgpi6vucatlb4sAAAAASEcwRAIgWPb8fGoz4bMVSNSByCbAFb0wE1qtQs1neQ2rZtKtJDsCIEoc7SYExnNbY5PltBaR3XiwDwxZQvufdRhW+qk4FX26Af7///8CgPD6AgAAAAAXqRQPuUY0IWlrgsgzryQceMF9295JNIfQ8gonAQAAABepFCnKdPigj4GZlCgYXJe12FLkBj9hh2UAAAABAwQBAAAAAQRHUiEClYO/Oa4KYJdHrRma3dY0+mEIVZ1sXNObTCGD8auW4H8hAtq2H/SaFNtqfQKwzR+7ePxLGDErW05U2uTbovv+9TbXUq4iBgKVg785rgpgl0etGZrd1jT6YQhVnWxc05tMIYPxq5bgfxDZDGpPAAAAgAAAAIAAAACAIgYC2rYf9JoU22p9ArDNH7t4/EsYMStbTlTa5Nui+/71NtcQ2QxqTwAAAIAAAACAAQAAgAABASAAwusLAAAAABepFLf1+vQOPUClpFmx2zU18rcvqSHohwEDBAEAAAABBCIAIIwjUxc3Q7WV37Sge3K6jkLjeX2nTof+fZ10l+OyAokDAQVHUiEDCJ3BDHrG21T5EymvYXMz2ziM6tDCMfcjN50bmQMLAtwhAjrdkE89bc9Z3bkGsN7iNSm3/7ntUOXoYVGSaGAiHw5zUq4iBgI63ZBPPW3PWd25BrDe4jUpt/+57VDl6GFRkmhgIh8OcxDZDGpPAAAAgAAAAIADAACAIgYDCJ3BDHrG21T5EymvYXMz2ziM6tDCMfcjN50bmQMLAtwQ2QxqTwAAAIAAAACAAgAAgAAiAgOppMN/WZbTqiXbrGtXCvBlA5RJKUJGCzVHU+2e7KWHcRDZDGpPAAAAgAAAAIAEAACAACICAn9jmXV9Lv9VoTatAsaEsYOLZVbl8bazQoKpS2tQBRCWENkMak8AAACAAAAAgAUAAIAA",
    ""
  },
  {
    "70736274ff01009a020000000258e87a21b56daf0c23be8e7070456c336f7cbaa5c8757924f545887bb2abdd750000000000ffffffff838d0427d0ec650a68aa46bb0b098aea4422c071b2ca78352a077959d07cea1d0100000000ffffffff0270aaf00800000000160014d85c2b71d0060b09c9886aeb815e50991dda124d00e1f5050000000016001400aea9a2e5f0f876a588df5546e8742d1d87008f00000000000100bb0200000001aad73931018bd25f84ae400b68848be09db706eac2ac18298babee71ab656f8b0000000048473044022058f6fc7c6a33e1b31548d481c826c015bd30135aad42cd67790dab66d2ad243b02204a1ced2604c6735b6393e5b41691dd78b00f0c5942fb9f751856faa938157dba01feffffff0280f0fa020000000017a9140fb9463421696b82c833af241c78c17ddbde493487d0f20a270100000017a91429ca74f8a08f81999428185c97b5d852e4063f6187650000002202029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f473044022074018ad4180097b873323c0015720b3684cc8123891048e7dbcd9b55ad679c99022073d369b740e3eb53dcefa33823c8070514ca55a7dd9544f157c167913261118c01010304010000000104475221029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f2102dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d752ae2206029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f10d90c6a4f000000800000008000000080220602dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d710d90c6a4f0000008000000080010000800001012000c2eb0b0000000017a914b7f5faf40e3d40a5a459b1db3535f2b72fa921e887220203089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc473044022062eb7a556107a7c73f45ac4ab5a1dddf6f7075fb1275969a7f383efff784bcb202200c05dbb7470dbf2f08557dd356c7325c1ed30913e996cd3840945db12228da5f010103040100000001042200208c2353173743b595dfb4a07b72ba8e42e3797da74e87fe7d9d7497e3b2028903010547522103089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc21023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7352ae2206023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7310d90c6a4f000000800000008003000080220603089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc10d90c6a4f00000080000000800200008000220203a9a4c37f5996d3aa25dbac6b570af0650394492942460b354753ed9eeca5877110d90c6a4f000000800000008004000080002202027f6399757d2eff55a136ad02c684b1838b6556e5f1b6b34282a94b6b5005109610d90c6a4f00000080000000800500008000",
    "cHNidP8BAJoCAAAAAljoeiG1ba8MI76OcHBFbDNvfLqlyHV5JPVFiHuyq911AAAAAAD/////g40EJ9DsZQpoqka7CwmK6kQiwHGyyng1Kgd5WdB86h0BAAAAAP////8CcKrwCAAAAAAWABTYXCtx0AYLCcmIauuBXlCZHdoSTQDh9QUAAAAAFgAUAK6pouXw+HaliN9VRuh0LR2HAI8AAAAAAAEAuwIAAAABqtc5MQGL0l+ErkALaISL4J23BurCrBgpi6vucatlb4sAAAAASEcwRAIgWPb8fGoz4bMVSNSByCbAFb0wE1qtQs1neQ2rZtKtJDsCIEoc7SYExnNbY5PltBaR3XiwDwxZQvufdRhW+qk4FX26Af7///8CgPD6AgAAAAAXqRQPuUY0IWlrgsgzryQceMF9295JNIfQ8gonAQAAABepFCnKdPigj4GZlCgYXJe12FLkBj9hh2UAAAAiAgKVg785rgpgl0etGZrd1jT6YQhVnWxc05tMIYPxq5bgf0cwRAIgdAGK1BgAl7hzMjwAFXILNoTMgSOJEEjn282bVa1nnJkCIHPTabdA4+tT3O+jOCPIBwUUylWn3ZVE8VfBZ5EyYRGMAQEDBAEAAAABBEdSIQKVg785rgpgl0etGZrd1jT6YQhVnWxc05tMIYPxq5bgfyEC2rYf9JoU22p9ArDNH7t4/EsYMStbTlTa5Nui+/71NtdSriIGApWDvzmuCmCXR60Zmt3WNPphCFWdbFzTm0whg/GrluB/ENkMak8AAACAAAAAgAAAAIAiBgLath/0mhTban0CsM0fu3j8SxgxK1tOVNrk26L7/vU21xDZDGpPAAAAgAAAAIABAACAAAEBIADC6wsAAAAAF6kUt/X69A49QKWkWbHbNTXyty+pIeiHIgIDCJ3BDHrG21T5EymvYXMz2ziM6tDCMfcjN50bmQMLAtxHMEQCIGLrelVhB6fHP0WsSrWh3d9vcHX7EnWWmn84Pv/3hLyyAiAMBdu3Rw2/LwhVfdNWxzJcHtMJE+mWzThAlF2xIijaXwEBAwQBAAAAAQQiACCMI1MXN0O1ld+0oHtyuo5C43l9p06H/n2ddJfjsgKJAwEFR1IhAwidwQx6xttU+RMpr2FzM9s4jOrQwjH3IzedG5kDCwLcIQI63ZBPPW3PWd25BrDe4jUpt/+57VDl6GFRkmhgIh8Oc1KuIgYCOt2QTz1tz1nduQaw3uI1Kbf/ue1Q5ehhUZJoYCIfDnMQ2QxqTwAAAIAAAACAAwAAgCIGAwidwQx6xttU+RMpr2FzM9s4jOrQwjH3IzedG5kDCwLcENkMak8AAACAAAAAgAIAAIAAIgIDqaTDf1mW06ol26xrVwrwZQOUSSlCRgs1R1Ptnuylh3EQ2QxqTwAAAIAAAACABAAAgAAiAgJ/Y5l1fS7/VaE2rQLGhLGDi2VW5fG2s0KCqUtrUAUQlhDZDGpPAAAAgAAAAIAFAACAAA==",
    ""
  },
  {
    "70736274ff01009a020000000258e87a21b56daf0c23be8e7070456c336f7cbaa5c8757924f545887bb2abdd750000000000ffffffff838d0427d0ec650a68aa46bb0b098aea4422c071b2ca78352a077959d07cea1d0100000000ffffffff0270aaf00800000000160014d85c2b71d0060b09c9886aeb815e50991dda124d00e1f5050000000016001400aea9a2e5f0f876a588df5546e8742d1d87008f00000000000100bb0200000001aad73931018bd25f84ae400b68848be09db706eac2ac18298babee71ab656f8b0000000048473044022058f6fc7c6a33e1b31548d481c826c015bd30135aad42cd67790dab66d2ad243b02204a1ced2604c6735b6393e5b41691dd78b00f0c5942fb9f751856faa938157dba01feffffff0280f0fa020000000017a9140fb9463421696b82c833af241c78c17ddbde493487d0f20a270100000017a91429ca74f8a08f81999428185c97b5d852e4063f618765000000220202dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d7483045022100f61038b308dc1da865a34852746f015772934208c6d24454393cd99bdf2217770220056e675a675a6d0a02b85b14e5e29074d8a25a9b5760bea2816f661910a006ea01010304010000000104475221029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f2102dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d752ae2206029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f10d90c6a4f000000800000008000000080220602dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d710d90c6a4f0000008000000080010000800001012000c2eb0b0000000017a914b7f5faf40e3d40a5a459b1db3535f2b72fa921e8872202023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e73473044022065f45ba5998b59a27ffe1a7bed016af1f1f90d54b3aa8f7450aa5f56a25103bd02207f724703ad1edb96680b284b56d4ffcb88f7fb759eabbe08aa30f29b851383d2010103040100000001042200208c2353173743b595dfb4a07b72ba8e42e3797da74e87fe7d9d7497e3b2028903010547522103089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc21023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7352ae2206023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7310d90c6a4f000000800000008003000080220603089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc10d90c6a4f00000080000000800200008000220203a9a4c37f5996d3aa25dbac6b570af0650394492942460b354753ed9eeca5877110d90c6a4f000000800000008004000080002202027f6399757d2eff55a136ad02c684b1838b6556e5f1b6b34282a94b6b5005109610d90c6a4f00000080000000800500008000",
    "cHNidP8BAJoCAAAAAljoeiG1ba8MI76OcHBFbDNvfLqlyHV5JPVFiHuyq911AAAAAAD/////g40EJ9DsZQpoqka7CwmK6kQiwHGyyng1Kgd5WdB86h0BAAAAAP////8CcKrwCAAAAAAWABTYXCtx0AYLCcmIauuBXlCZHdoSTQDh9QUAAAAAFgAUAK6pouXw+HaliN9VRuh0LR2HAI8AAAAAAAEAuwIAAAABqtc5MQGL0l+ErkALaISL4J23BurCrBgpi6vucatlb4sAAAAASEcwRAIgWPb8fGoz4bMVSNSByCbAFb0wE1qtQs1neQ2rZtKtJDsCIEoc7SYExnNbY5PltBaR3XiwDwxZQvufdRhW+qk4FX26Af7///8CgPD6AgAAAAAXqRQPuUY0IWlrgsgzryQceMF9295JNIfQ8gonAQAAABepFCnKdPigj4GZlCgYXJe12FLkBj9hh2UAAAAiAgLath/0mhTban0CsM0fu3j8SxgxK1tOVNrk26L7/vU210gwRQIhAPYQOLMI3B2oZaNIUnRvAVdyk0IIxtJEVDk82ZvfIhd3AiAFbmdaZ1ptCgK4WxTl4pB02KJam1dgvqKBb2YZEKAG6gEBAwQBAAAAAQRHUiEClYO/Oa4KYJdHrRma3dY0+mEIVZ1sXNObTCGD8auW4H8hAtq2H/SaFNtqfQKwzR+7ePxLGDErW05U2uTbovv+9TbXUq4iBgKVg785rgpgl0etGZrd1jT6YQhVnWxc05tMIYPxq5bgfxDZDGpPAAAAgAAAAIAAAACAIgYC2rYf9JoU22p9ArDNH7t4/EsYMStbTlTa5Nui+/71NtcQ2QxqTwAAAIAAAACAAQAAgAABASAAwusLAAAAABepFLf1+vQOPUClpFmx2zU18rcvqSHohyICAjrdkE89bc9Z3bkGsN7iNSm3/7ntUOXoYVGSaGAiHw5zRzBEAiBl9FulmYtZon/+GnvtAWrx8fkNVLOqj3RQql9WolEDvQIgf3JHA60e25ZoCyhLVtT/y4j3+3Weq74IqjDym4UTg9IBAQMEAQAAAAEEIgAgjCNTFzdDtZXftKB7crqOQuN5fadOh/59nXSX47ICiQMBBUdSIQMIncEMesbbVPkTKa9hczPbOIzq0MIx9yM3nRuZAwsC3CECOt2QTz1tz1nduQaw3uI1Kbf/ue1Q5ehhUZJoYCIfDnNSriIGAjrdkE89bc9Z3bkGsN7iNSm3/7ntUOXoYVGSaGAiHw5zENkMak8AAACAAAAAgAMAAIAiBgMIncEMesbbVPkTKa9hczPbOIzq0MIx9yM3nRuZAwsC3BDZDGpPAAAAgAAAAIACAACAACICA6mkw39ZltOqJdusa1cK8GUDlEkpQkYLNUdT7Z7spYdxENkMak8AAACAAAAAgAQAAIAAIgICf2OZdX0u/1WhNq0CxoSxg4tlVuXxtrNCgqlLa1AFEJYQ2QxqTwAAAIAAAACABQAAgAA=",
    ""
  },
  {
    "70736274ff01009a020000000258e87a21b56daf0c23be8e7070456c336f7cbaa5c8757924f545887bb2abdd750000000000ffffffff838d0427d0ec650a68aa46bb0b098aea4422c071b2ca78352a077959d07cea1d0100000000ffffffff0270aaf00800000000160014d85c2b71d0060b09c9886aeb815e50991dda124d00e1f5050000000016001400aea9a2e5f0f876a588df5546e8742d1d87008f00000000000100bb0200000001aad73931018bd25f84ae400b68848be09db706eac2ac18298babee71ab656f8b0000000048473044022058f6fc7c6a33e1b31548d481c826c015bd30135aad42cd67790dab66d2ad243b02204a1ced2604c6735b6393e5b41691dd78b00f0c5942fb9f751856faa938157dba01feffffff0280f0fa020000000017a9140fb9463421696b82c833af241c78c17ddbde493487d0f20a270100000017a91429ca74f8a08f81999428185c97b5d852e4063f6187650000002202029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f473044022074018ad4180097b873323c0015720b3684cc8123891048e7dbcd9b55ad679c99022073d369b740e3eb53dcefa33823c8070514ca55a7dd9544f157c167913261118c01220202dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d7483045022100f61038b308dc1da865a34852746f015772934208c6d24454393cd99bdf2217770220056e675a675a6d0a02b85b14e5e29074d8a25a9b5760bea2816f661910a006ea01010304010000000104475221029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f2102dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d752ae2206029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f10d90c6a4f000000800000008000000080220602dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d710d90c6a4f0000008000000080010000800001012000c2eb0b0000000017a914b7f5faf40e3d40a5a459b1db3535f2b72fa921e887220203089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc473044022062eb7a556107a7c73f45ac4ab5a1dddf6f7075fb1275969a7f383efff784bcb202200c05dbb7470dbf2f08557dd356c7325c1ed30913e996cd3840945db12228da5f012202023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e73473044022065f45ba5998b59a27ffe1a7bed016af1f1f90d54b3aa8f7450aa5f56a25103bd02207f724703ad1edb96680b284b56d4ffcb88f7fb759eabbe08aa30f29b851383d2010103040100000001042200208c2353173743b595dfb4a07b72ba8e42e3797da74e87fe7d9d7497e3b2028903010547522103089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc21023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7352ae2206023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7310d90c6a4f000000800000008003000080220603089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc10d90c6a4f00000080000000800200008000220203a9a4c37f5996d3aa25dbac6b570af0650394492942460b354753ed9eeca5877110d90c6a4f000000800000008004000080002202027f6399757d2eff55a136ad02c684b1838b6556e5f1b6b34282a94b6b5005109610d90c6a4f00000080000000800500008000",
    "cHNidP8BAJoCAAAAAljoeiG1ba8MI76OcHBFbDNvfLqlyHV5JPVFiHuyq911AAAAAAD/////g40EJ9DsZQpoqka7CwmK6kQiwHGyyng1Kgd5WdB86h0BAAAAAP////8CcKrwCAAAAAAWABTYXCtx0AYLCcmIauuBXlCZHdoSTQDh9QUAAAAAFgAUAK6pouXw+HaliN9VRuh0LR2HAI8AAAAAAAEAuwIAAAABqtc5MQGL0l+ErkALaISL4J23BurCrBgpi6vucatlb4sAAAAASEcwRAIgWPb8fGoz4bMVSNSByCbAFb0wE1qtQs1neQ2rZtKtJDsCIEoc7SYExnNbY5PltBaR3XiwDwxZQvufdRhW+qk4FX26Af7///8CgPD6AgAAAAAXqRQPuUY0IWlrgsgzryQceMF9295JNIfQ8gonAQAAABepFCnKdPigj4GZlCgYXJe12FLkBj9hh2UAAAAiAgKVg785rgpgl0etGZrd1jT6YQhVnWxc05tMIYPxq5bgf0cwRAIgdAGK1BgAl7hzMjwAFXILNoTMgSOJEEjn282bVa1nnJkCIHPTabdA4+tT3O+jOCPIBwUUylWn3ZVE8VfBZ5EyYRGMASICAtq2H/SaFNtqfQKwzR+7ePxLGDErW05U2uTbovv+9TbXSDBFAiEA9hA4swjcHahlo0hSdG8BV3KTQgjG0kRUOTzZm98iF3cCIAVuZ1pnWm0KArhbFOXikHTYolqbV2C+ooFvZhkQoAbqAQEDBAEAAAABBEdSIQKVg785rgpgl0etGZrd1jT6YQhVnWxc05tMIYPxq5bgfyEC2rYf9JoU22p9ArDNH7t4/EsYMStbTlTa5Nui+/71NtdSriIGApWDvzmuCmCXR60Zmt3WNPphCFWdbFzTm0whg/GrluB/ENkMak8AAACAAAAAgAAAAIAiBgLath/0mhTban0CsM0fu3j8SxgxK1tOVNrk26L7/vU21xDZDGpPAAAAgAAAAIABAACAAAEBIADC6wsAAAAAF6kUt/X69A49QKWkWbHbNTXyty+pIeiHIgIDCJ3BDHrG21T5EymvYXMz2ziM6tDCMfcjN50bmQMLAtxHMEQCIGLrelVhB6fHP0WsSrWh3d9vcHX7EnWWmn84Pv/3hLyyAiAMBdu3Rw2/LwhVfdNWxzJcHtMJE+mWzThAlF2xIijaXwEiAgI63ZBPPW3PWd25BrDe4jUpt/+57VDl6GFRkmhgIh8Oc0cwRAIgZfRbpZmLWaJ//hp77QFq8fH5DVSzqo90UKpfVqJRA70CIH9yRwOtHtuWaAsoS1bU/8uI9/t1nqu+CKow8puFE4PSAQEDBAEAAAABBCIAIIwjUxc3Q7WV37Sge3K6jkLjeX2nTof+fZ10l+OyAokDAQVHUiEDCJ3BDHrG21T5EymvYXMz2ziM6tDCMfcjN50bmQMLAtwhAjrdkE89bc9Z3bkGsN7iNSm3/7ntUOXoYVGSaGAiHw5zUq4iBgI63ZBPPW3PWd25BrDe4jUpt/+57VDl6GFRkmhgIh8OcxDZDGpPAAAAgAAAAIADAACAIgYDCJ3BDHrG21T5EymvYXMz2ziM6tDCMfcjN50bmQMLAtwQ2QxqTwAAAIAAAACAAgAAgAAiAgOppMN/WZbTqiXbrGtXCvBlA5RJKUJGCzVHU+2e7KWHcRDZDGpPAAAAgAAAAIAEAACAACICAn9jmXV9Lv9VoTatAsaEsYOLZVbl8bazQoKpS2tQBRCWENkMak8AAACAAAAAgAUAAIAA",
    ""
  },
  {
    "70736274ff01009a020000000258e87a21b56daf0c23be8e7070456c336f7cbaa5c8757924f545887bb2abdd750000000000ffffffff838d0427d0ec650a68aa46bb0b098aea4422c071b2ca78352a077959d07cea1d0100000000ffffffff0270aaf00800000000160014d85c2b71d0060b09c9886aeb815e50991dda124d00e1f5050000000016001400aea9a2e5f0f876a588df5546e8742d1d87008f00000000000100bb0200000001aad73931018bd25f84ae400b68848be09db706eac2ac18298babee71ab656f8b0000000048473044022058f6fc7c6a33e1b31548d481c826c015bd30135aad42cd67790dab66d2ad243b02204a1ced2604c6735b6393e5b41691dd78b00f0c5942fb9f751856faa938157dba01feffffff0280f0fa020000000017a9140fb9463421696b82c833af241c78c17ddbde493487d0f20a270100000017a91429ca74f8a08f81999428185c97b5d852e4063f6187650000000107da00473044022074018ad4180097b873323c0015720b3684cc8123891048e7dbcd9b55ad679c99022073d369b740e3eb53dcefa33823c8070514ca55a7dd9544f157c167913261118c01483045022100f61038b308dc1da865a34852746f015772934208c6d24454393cd99bdf2217770220056e675a675a6d0a02b85b14e5e29074d8a25a9b5760bea2816f661910a006ea01475221029583bf39ae0a609747ad199addd634fa6108559d6c5cd39b4c2183f1ab96e07f2102dab61ff49a14db6a7d02b0cd1fbb78fc4b18312b5b4e54dae4dba2fbfef536d752ae0001012000c2eb0b0000000017a914b7f5faf40e3d40a5a459b1db3535f2b72fa921e8870107232200208c2353173743b595dfb4a07b72ba8e42e3797da74e87fe7d9d7497e3b20289030108da0400473044022062eb7a556107a7c73f45ac4ab5a1dddf6f7075fb1275969a7f383efff784bcb202200c05dbb7470dbf2f08557dd356c7325c1ed30913e996cd3840945db12228da5f01473044022065f45ba5998b59a27ffe1a7bed016af1f1f90d54b3aa8f7450aa5f56a25103bd02207f724703ad1edb96680b284b56d4ffcb88f7fb759eabbe08aa30f29b851383d20147522103089dc10c7ac6db54f91329af617333db388cead0c231f723379d1b99030b02dc21023add904f3d6dcf59ddb906b0dee23529b7ffb9ed50e5e86151926860221f0e7352ae00220203a9a4c37f5996d3aa25dbac6b570af0650394492942460b354753ed9eeca5877110d90c6a4f000000800000008004000080002202027f6399757d2eff55a136ad02c684b1838b6556e5f1b6b34282a94b6b5005109610d90c6a4f00000080000000800500008000",
    "cHNidP8BAJoCAAAAAljoeiG1ba8MI76OcHBFbDNvfLqlyHV5JPVFiHuyq911AAAAAAD/////g40EJ9DsZQpoqka7CwmK6kQiwHGyyng1Kgd5WdB86h0BAAAAAP////8CcKrwCAAAAAAWABTYXCtx0AYLCcmIauuBXlCZHdoSTQDh9QUAAAAAFgAUAK6pouXw+HaliN9VRuh0LR2HAI8AAAAAAAEAuwIAAAABqtc5MQGL0l+ErkALaISL4J23BurCrBgpi6vucatlb4sAAAAASEcwRAIgWPb8fGoz4bMVSNSByCbAFb0wE1qtQs1neQ2rZtKtJDsCIEoc7SYExnNbY5PltBaR3XiwDwxZQvufdRhW+qk4FX26Af7///8CgPD6AgAAAAAXqRQPuUY0IWlrgsgzryQceMF9295JNIfQ8gonAQAAABepFCnKdPigj4GZlCgYXJe12FLkBj9hh2UAAAABB9oARzBEAiB0AYrUGACXuHMyPAAVcgs2hMyBI4kQSOfbzZtVrWecmQIgc9Npt0Dj61Pc76M4I8gHBRTKVafdlUTxV8FnkTJhEYwBSDBFAiEA9hA4swjcHahlo0hSdG8BV3KTQgjG0kRUOTzZm98iF3cCIAVuZ1pnWm0KArhbFOXikHTYolqbV2C+ooFvZhkQoAbqAUdSIQKVg785rgpgl0etGZrd1jT6YQhVnWxc05tMIYPxq5bgfyEC2rYf9JoU22p9ArDNH7t4/EsYMStbTlTa5Nui+/71NtdSrgABASAAwusLAAAAABepFLf1+vQOPUClpFmx2zU18rcvqSHohwEHIyIAIIwjUxc3Q7WV37Sge3K6jkLjeX2nTof+fZ10l+OyAokDAQjaBABHMEQCIGLrelVhB6fHP0WsSrWh3d9vcHX7EnWWmn84Pv/3hLyyAiAMBdu3Rw2/LwhVfdNWxzJcHtMJE+mWzThAlF2xIijaXwFHMEQCIGX0W6WZi1mif/4ae+0BavHx+Q1Us6qPdFCqX1aiUQO9AiB/ckcDrR7blmgLKEtW1P/LiPf7dZ6rvgiqMPKbhROD0gFHUiEDCJ3BDHrG21T5EymvYXMz2ziM6tDCMfcjN50bmQMLAtwhAjrdkE89bc9Z3bkGsN7iNSm3/7ntUOXoYVGSaGAiHw5zUq4AIgIDqaTDf1mW06ol26xrVwrwZQOUSSlCRgs1R1Ptnuylh3EQ2QxqTwAAAIAAAACABAAAgAAiAgJ/Y5l1fS7/VaE2rQLGhLGDi2VW5fG2s0KCqUtrUAUQlhDZDGpPAAAAgAAAAIAFAACAAA==",
    ""
  },
  {
    "70736274ff01003f0200000001ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff0000000000ffffffff010000000000000000036a0100000000000a0f0102030405060708090f0102030405060708090a0b0c0d0e0f000a0f0102030405060708090f0102030405060708090a0b0c0d0e0f000a0f0102030405060708090f0102030405060708090a0b0c0d0e0f00",
    "cHNidP8BAD8CAAAAAf//////////////////////////////////////////AAAAAAD/////AQAAAAAAAAAAA2oBAAAAAAAKDwECAwQFBgcICQ8BAgMEBQYHCAkKCwwNDg8ACg8BAgMEBQYHCAkPAQIDBAUGBwgJCgsMDQ4PAAoPAQIDBAUGBwgJDwECAwQFBgcICQoLDA0ODwA=",
    ""
  },
  {
    "70736274ff01003f0200000001ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff0000000000ffffffff010000000000000000036a0100000000000a0f0102030405060708100f0102030405060708090a0b0c0d0e0f000a0f0102030405060708100f0102030405060708090a0b0c0d0e0f000a0f0102030405060708100f0102030405060708090a0b0c0d0e0f00",
    "cHNidP8BAD8CAAAAAf//////////////////////////////////////////AAAAAAD/////AQAAAAAAAAAAA2oBAAAAAAAKDwECAwQFBgcIEA8BAgMEBQYHCAkKCwwNDg8ACg8BAgMEBQYHCBAPAQIDBAUGBwgJCgsMDQ4PAAoPAQIDBAUGBwgQDwECAwQFBgcICQoLDA0ODwA=",
    ""
  },
  {
    "70736274ff01003f0200000001ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff0000000000ffffffff010000000000000000036a0100000000000a0f0102030405060708090f0102030405060708090a0b0c0d0e0f0a0f0102030405060708100f0102030405060708090a0b0c0d0e0f000a0f0102030405060708090f0102030405060708090a0b0c0d0e0f0a0f0102030405060708100f0102030405060708090a0b0c0d0e0f000a0f0102030405060708090f0102030405060708090a0b0c0d0e0f0a0f0102030405060708100f0102030405060708090a0b0c0d0e0f00",
    "cHNidP8BAD8CAAAAAf//////////////////////////////////////////AAAAAAD/////AQAAAAAAAAAAA2oBAAAAAAAKDwECAwQFBgcICQ8BAgMEBQYHCAkKCwwNDg8KDwECAwQFBgcIEA8BAgMEBQYHCAkKCwwNDg8ACg8BAgMEBQYHCAkPAQIDBAUGBwgJCgsMDQ4PCg8BAgMEBQYHCBAPAQIDBAUGBwgJCgsMDQ4PAAoPAQIDBAUGBwgJDwECAwQFBgcICQoLDA0ODwoPAQIDBAUGBwgQDwECAwQFBgcICQoLDA0ODwA=",
    ""
  }
};

TEST(Psbt, ParsePsbt) {
  for (const auto& data : g_cfd_psbt_testdata) {
    {
      SCOPED_TRACE("hex[" + data.hex + "]");
      try {
        Psbt from_b64(data.base64);
        if (data.error_message.empty()) {
          EXPECT_EQ(data.hex, from_b64.GetData().GetHex());
          EXPECT_EQ(data.base64, from_b64.GetBase64());
        } else {
          EXPECT_STREQ("The current test should generate an error.", data.hex.c_str());
        }
      } catch (const CfdException& except) {
        EXPECT_STREQ(data.error_message.c_str(), except.what());
      }
    }
  }
}

TEST(Psbt, SetTxInOnly) {
  Psbt psbt;
  EXPECT_EQ(0, psbt.GetTxInCount());
  TxIn txin_w(Txid("c078957064d70a5e80c3c23302528524d424aaabce86fb3fc1b6e6ea76fd7f26"), 1, 0xffffffff);
  TxIn txin_l(Txid("c0ecc9313d16355d71b96acff5bca43cdb593289e50154bab12cff422a46257d"), 0, 0xffffffff);
  TxInReference txin_r(txin_l);
  psbt.AddTxIn(txin_w);
  psbt.AddTxIn(txin_r);
  EXPECT_EQ(2, psbt.GetTxInCount());

  EXPECT_STREQ("70736274ff01005c0200000002267ffd76eae6b6c13ffb86ceabaa24d42485520233c2c3805e0ad764709578c00100000000ffffffff7d25462a42ff2cb1ba5401e5893259db3ca4bcf5cf6ab9715d35163d31c9ecc00000000000ffffffff0000000000000000", psbt.GetData().GetHex().c_str());
  EXPECT_STREQ("cHNidP8BAFwCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8AAAAAAAAAAA==", psbt.GetBase64().c_str());
}

TEST(Psbt, SetInputOnly) {
  Psbt psbt("cHNidP8BAFwCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8AAAAAAAAAAA==");

  HDWallet wallet1 = HDWallet(ByteData(g_psbt_seed1));
  HDWallet wallet2 = HDWallet(ByteData(g_psbt_seed2));
  std::string path1 = "44h/0h/0h/1/1";
  std::string path2 = "44h/0h/0h/0/1";

  auto key1 = wallet1.GeneratePubkeyData(NetType::kTestnet, path1);
  auto key2 = wallet2.GeneratePubkeyData(NetType::kTestnet, path2);
  try {
    psbt.SetTxInUtxo(0, Transaction(g_psbt_utxo_witness), key1);
    psbt.SetTxInSighashType(0, SigHashType());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
  }

  try {
    psbt.SetTxInUtxo(1, Transaction(g_psbt_utxo_legacy), key2);
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
  }

  EXPECT_TRUE(psbt.IsFindTxInSighashType(0));
  EXPECT_FALSE(psbt.IsFindTxInSighashType(1));
  EXPECT_EQ(SigHashType().GetSigHashFlag(), psbt.GetTxInSighashType(0).GetSigHashFlag());

  EXPECT_STREQ("70736274ff01005c0200000002267ffd76eae6b6c13ffb86ceabaa24d42485520233c2c3805e0ad764709578c00100000000ffffffff7d25462a42ff2cb1ba5401e5893259db3ca4bcf5cf6ab9715d35163d31c9ecc00000000000ffffffff0000000000000100f602000000000101f1993fe8e7189542ee4506258e170201be292703cd275acb09ece16672fd848b0000000017160014ac9ef80b27af1c9d95c1db5d761319322bc42fc5ffffffff02080410240100000016001409de2a0431cbb3444fc22cad9d9a0fd09639721000e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac975307870247304402201e07df721c3322419e8f36d07eeae4795975ba0d9d19630ca3cd3dc0d4967172022015428e7be06b6567501539050bd791a380f00bbddbc5097ea97ba7be4017114a0121024aef43b1d5ac7ba5014998d63ceac583959d1fdc66ea2699cd84eeaf82a283060000000001012000e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac97530787010304010000000104160014962c4e08f336d3afbc3415c9d359ae1040470520220602565248460b3c186decf13db06f0724fd50b47cc3e489c30076c8363d5038cefe182a7047602c00008000000080000000800100000001000000000100bf0200000001c6d2ea36e2e802b52ddac665dacbed2f831b5263459e1ca734f5c945d7515e40000000006a473044022011b96c7d2d0d2e8dcb37138e18acc460752965add9cb8787df5c349df0e2ae6602202e93af31b64f5166e5605819555dabec57be794300fadb37052f31dddea9905c012103e3d244a3967e0b87765fda86c5ff38885f74993953b9584388aef30b26af6aecffffffff017851cd1d000000001976a9148d20443a91969e3bca0e240cd0ffe4dc98c63de288ac00000000220602d9f6888f285a15a6a1880a2202c2b230a42d77cf62da6a48aca4198262cda3c7189d6b6d862c0000800000008000000080000000000100000000", psbt.GetData().GetHex().c_str());
  Psbt psbt2(psbt.GetData());
  EXPECT_STREQ("cHNidP8BAFwCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8AAAAAAAABAPYCAAAAAAEB8Zk/6OcYlULuRQYljhcCAb4pJwPNJ1rLCezhZnL9hIsAAAAAFxYAFKye+AsnrxydlcHbXXYTGTIrxC/F/////wIIBBAkAQAAABYAFAneKgQxy7NET8IsrZ2aD9CWOXIQAOH1BQAAAAAXqRRQn1mF9OkKFPuQ45MW/bTzrJdTB4cCRzBEAiAeB99yHDMiQZ6PNtB+6uR5WXW6DZ0ZYwyjzT3A1JZxcgIgFUKOe+BrZWdQFTkFC9eRo4DwC73bxQl+qXunvkAXEUoBIQJK70Ox1ax7pQFJmNY86sWDlZ0f3GbqJpnNhO6vgqKDBgAAAAABASAA4fUFAAAAABepFFCfWYX06QoU+5Djkxb9tPOsl1MHhwEDBAEAAAABBBYAFJYsTgjzNtOvvDQVydNZrhBARwUgIgYCVlJIRgs8GG3s8T2wbwck/VC0fMPkicMAdsg2PVA4zv4YKnBHYCwAAIAAAACAAAAAgAEAAAABAAAAAAEAvwIAAAABxtLqNuLoArUt2sZl2svtL4MbUmNFnhynNPXJRddRXkAAAAAAakcwRAIgEblsfS0NLo3LNxOOGKzEYHUpZa3Zy4eH31w0nfDirmYCIC6TrzG2T1Fm5WBYGVVdq+xXvnlDAPrbNwUvMd3eqZBcASED49JEo5Z+C4d2X9qGxf84iF90mTlTuVhDiK7zCyavauz/////AXhRzR0AAAAAGXapFI0gRDqRlp47yg4kDND/5NyYxj3iiKwAAAAAIgYC2faIjyhaFaahiAoiAsKyMKQtd89i2mpIrKQZgmLNo8cYnWtthiwAAIAAAACAAAAAgAAAAAABAAAAAA==", psbt2.GetBase64().c_str());
}

TEST(Psbt, SetInputOnlySimpleWitness) {
  Psbt psbt("cHNidP8BAFwCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8AAAAAAAAAAA==");

  HDWallet wallet1 = HDWallet(ByteData(g_psbt_seed1));
  HDWallet wallet2 = HDWallet(ByteData(g_psbt_seed2));
  std::string path1 = "44h/0h/0h/1/1";
  std::string path2 = "44h/0h/0h/0/1";

  auto key1 = wallet1.GeneratePubkeyData(NetType::kTestnet, path1);
  auto key2 = wallet2.GeneratePubkeyData(NetType::kTestnet, path2);
  try {
    auto tx = psbt.GetTransaction();
    psbt.SetTxInUtxo(0, Transaction(g_psbt_utxo_witness).GetTxOut(tx.GetTxIn(0).GetVout()), key1);
    psbt.SetTxInSighashType(0, SigHashType());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
  }

  try {
    psbt.SetTxInUtxo(1, Transaction(g_psbt_utxo_legacy), key2);
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
  }

  EXPECT_TRUE(psbt.IsFindTxInSighashType(0));
  EXPECT_FALSE(psbt.IsFindTxInSighashType(1));
  EXPECT_EQ(SigHashType().GetSigHashFlag(), psbt.GetTxInSighashType(0).GetSigHashFlag());

  EXPECT_STREQ("70736274ff01005c0200000002267ffd76eae6b6c13ffb86ceabaa24d42485520233c2c3805e0ad764709578c00100000000ffffffff7d25462a42ff2cb1ba5401e5893259db3ca4bcf5cf6ab9715d35163d31c9ecc00000000000ffffffff00000000000001012000e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac97530787010304010000000104160014962c4e08f336d3afbc3415c9d359ae1040470520220602565248460b3c186decf13db06f0724fd50b47cc3e489c30076c8363d5038cefe182a7047602c00008000000080000000800100000001000000000100bf0200000001c6d2ea36e2e802b52ddac665dacbed2f831b5263459e1ca734f5c945d7515e40000000006a473044022011b96c7d2d0d2e8dcb37138e18acc460752965add9cb8787df5c349df0e2ae6602202e93af31b64f5166e5605819555dabec57be794300fadb37052f31dddea9905c012103e3d244a3967e0b87765fda86c5ff38885f74993953b9584388aef30b26af6aecffffffff017851cd1d000000001976a9148d20443a91969e3bca0e240cd0ffe4dc98c63de288ac00000000220602d9f6888f285a15a6a1880a2202c2b230a42d77cf62da6a48aca4198262cda3c7189d6b6d862c0000800000008000000080000000000100000000", psbt.GetData().GetHex().c_str());
  Psbt psbt2(psbt.GetData());
  EXPECT_STREQ("cHNidP8BAFwCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8AAAAAAAABASAA4fUFAAAAABepFFCfWYX06QoU+5Djkxb9tPOsl1MHhwEDBAEAAAABBBYAFJYsTgjzNtOvvDQVydNZrhBARwUgIgYCVlJIRgs8GG3s8T2wbwck/VC0fMPkicMAdsg2PVA4zv4YKnBHYCwAAIAAAACAAAAAgAEAAAABAAAAAAEAvwIAAAABxtLqNuLoArUt2sZl2svtL4MbUmNFnhynNPXJRddRXkAAAAAAakcwRAIgEblsfS0NLo3LNxOOGKzEYHUpZa3Zy4eH31w0nfDirmYCIC6TrzG2T1Fm5WBYGVVdq+xXvnlDAPrbNwUvMd3eqZBcASED49JEo5Z+C4d2X9qGxf84iF90mTlTuVhDiK7zCyavauz/////AXhRzR0AAAAAGXapFI0gRDqRlp47yg4kDND/5NyYxj3iiKwAAAAAIgYC2faIjyhaFaahiAoiAsKyMKQtd89i2mpIrKQZgmLNo8cYnWtthiwAAIAAAACAAAAAgAAAAAABAAAAAA==", psbt2.GetBase64().c_str());
}

TEST(Psbt, SetTxOutOnly) {
  Psbt psbt;
  EXPECT_EQ(0, psbt.GetTxOutCount());
  
  HDWallet wallet1 = HDWallet(ByteData(g_psbt_seed1));
  HDWallet wallet2 = HDWallet(ByteData(g_psbt_seed2));
  std::string path1 = "44h/0h/0h/0/2";
  std::string path2 = "44h/0h/0h/0/2";
  auto key1 = wallet1.GeneratePubkeyData(NetType::kTestnet, path1);
  auto key2 = wallet2.GeneratePubkeyData(NetType::kTestnet, path2);
  auto addr1 = Address(NetType::kTestnet, WitnessVersion::kVersion0, key1.GetPubkey());
  auto addr2 = Address(NetType::kTestnet, WitnessVersion::kVersion0, key2.GetPubkey());

  Amount amount(100000000);
  TxOut txout_1(amount, addr1);
  TxOut txout_2(amount, addr2);
  TxOutReference txout_2r(txout_2);
  psbt.AddTxOut(txout_1);
  psbt.AddTxOut(txout_2r);
  EXPECT_EQ(2, psbt.GetTxOutCount());

  EXPECT_STREQ("70736274ff01004802000000000200e1f50500000000160014b322bddce633b851ac7370ab454f0b367a0654e500e1f50500000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a55500000000000000", psbt.GetData().GetHex().c_str());
  Psbt psbt2(psbt.GetData());
  EXPECT_STREQ("cHNidP8BAEgCAAAAAAIA4fUFAAAAABYAFLMivdzmM7hRrHNwq0VPCzZ6BlTlAOH1BQAAAAAWABTKuMU6bo/AKW0c05FaMH1RxJGlVQAAAAAAAAA=", psbt2.GetBase64().c_str());
}

TEST(Psbt, SetOutputOnly) {
  Psbt psbt("cHNidP8BAEgCAAAAAAIA4fUFAAAAABYAFLMivdzmM7hRrHNwq0VPCzZ6BlTlAOH1BQAAAAAWABTKuMU6bo/AKW0c05FaMH1RxJGlVQAAAAAAAAA=");

  HDWallet wallet1 = HDWallet(ByteData(g_psbt_seed1));
  HDWallet wallet2 = HDWallet(ByteData(g_psbt_seed2));
  std::string path1 = "44h/0h/0h/0/2";
  std::string path2 = "44h/0h/0h/0/2";
  auto key1 = wallet1.GeneratePubkeyData(NetType::kTestnet, path1);
  auto key2 = wallet2.GeneratePubkeyData(NetType::kTestnet, path2);
  auto addr1 = Address(NetType::kTestnet, WitnessVersion::kVersion0, key1.GetPubkey());
  auto addr2 = Address(NetType::kTestnet, WitnessVersion::kVersion0, key2.GetPubkey());

  Amount amount(100000000);
  try {
    psbt.SetTxOutData(0, key1);
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
  }

  try {
    psbt.SetTxOutData(1, key2);
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
  }

  EXPECT_STREQ("70736274ff01004802000000000200e1f50500000000160014b322bddce633b851ac7370ab454f0b367a0654e500e1f50500000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a5550000000000220203473bfc8c770c1b220a2e7aae4badf6c0d7eaf29028d5b29d3438012bb289ef81182a7047602c00008000000080000000800000000002000000002202036474aff2633c351865539fb52b62b9d6fb9e4e23576628e1f0a0a7993458e06c189d6b6d862c0000800000008000000080000000000200000000", psbt.GetData().GetHex().c_str());
  auto b64 = CryptoUtil::EncodeBase64(psbt.GetData());
  Psbt psbt2(psbt.GetData());
  EXPECT_STREQ("cHNidP8BAEgCAAAAAAIA4fUFAAAAABYAFLMivdzmM7hRrHNwq0VPCzZ6BlTlAOH1BQAAAAAWABTKuMU6bo/AKW0c05FaMH1RxJGlVQAAAAAAIgIDRzv8jHcMGyIKLnquS632wNfq8pAo1bKdNDgBK7KJ74EYKnBHYCwAAIAAAACAAAAAgAAAAAACAAAAACICA2R0r/JjPDUYZVOftStiudb7nk4jV2Yo4fCgp5k0WOBsGJ1rbYYsAACAAAAAgAAAAIAAAAAAAgAAAAA=", psbt.GetBase64().c_str());
  EXPECT_STREQ("cHNidP8BAEgCAAAAAAIA4fUFAAAAABYAFLMivdzmM7hRrHNwq0VPCzZ6BlTlAOH1BQAAAAAWABTKuMU6bo/AKW0c05FaMH1RxJGlVQAAAAAAIgIDRzv8jHcMGyIKLnquS632wNfq8pAo1bKdNDgBK7KJ74EYKnBHYCwAAIAAAACAAAAAgAAAAAACAAAAACICA2R0r/JjPDUYZVOftStiudb7nk4jV2Yo4fCgp5k0WOBsGJ1rbYYsAACAAAAAgAAAAIAAAAAAAgAAAAA=", psbt2.GetBase64().c_str());
  EXPECT_STREQ("cHNidP8BAEgCAAAAAAIA4fUFAAAAABYAFLMivdzmM7hRrHNwq0VPCzZ6BlTlAOH1BQAAAAAWABTKuMU6bo/AKW0c05FaMH1RxJGlVQAAAAAAIgIDRzv8jHcMGyIKLnquS632wNfq8pAo1bKdNDgBK7KJ74EYKnBHYCwAAIAAAACAAAAAgAAAAAACAAAAACICA2R0r/JjPDUYZVOftStiudb7nk4jV2Yo4fCgp5k0WOBsGJ1rbYYsAACAAAAAgAAAAIAAAAAAAgAAAAA=", b64.c_str());
}

TEST(Psbt, FromTransaction) {
  Transaction tx("0200000002267ffd76eae6b6c13ffb86ceabaa24d42485520233c2c3805e0ad764709578c00100000000ffffffff7d25462a42ff2cb1ba5401e5893259db3ca4bcf5cf6ab9715d35163d31c9ecc00000000000ffffffff0200e1f50500000000160014b322bddce633b851ac7370ab454f0b367a0654e500e1f50500000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a55500000000");
  Psbt psbt(tx);

  EXPECT_STREQ("70736274ff01009a0200000002267ffd76eae6b6c13ffb86ceabaa24d42485520233c2c3805e0ad764709578c00100000000ffffffff7d25462a42ff2cb1ba5401e5893259db3ca4bcf5cf6ab9715d35163d31c9ecc00000000000ffffffff0200e1f50500000000160014b322bddce633b851ac7370ab454f0b367a0654e500e1f50500000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a555000000000000000000", psbt.GetData().GetHex().c_str());
  Psbt psbt2(psbt.GetData());
  EXPECT_STREQ("cHNidP8BAJoCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8CAOH1BQAAAAAWABSzIr3c5jO4UaxzcKtFTws2egZU5QDh9QUAAAAAFgAUyrjFOm6PwCltHNORWjB9UcSRpVUAAAAAAAAAAAA=", psbt2.GetBase64().c_str());
}

TEST(Psbt, JoinTxOnly) {
  Psbt psbt;
  Psbt psbt_txin_only("cHNidP8BAFwCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8AAAAAAAAAAA==");
  Psbt psbt_txout_only("cHNidP8BAEgCAAAAAAIA4fUFAAAAABYAFLMivdzmM7hRrHNwq0VPCzZ6BlTlAOH1BQAAAAAWABTKuMU6bo/AKW0c05FaMH1RxJGlVQAAAAAAAAA=");

  psbt = psbt_txin_only;
  psbt.Join(psbt_txout_only, true);

  EXPECT_STREQ("70736274ff01009a0200000002267ffd76eae6b6c13ffb86ceabaa24d42485520233c2c3805e0ad764709578c00100000000ffffffff7d25462a42ff2cb1ba5401e5893259db3ca4bcf5cf6ab9715d35163d31c9ecc00000000000ffffffff0200e1f50500000000160014b322bddce633b851ac7370ab454f0b367a0654e500e1f50500000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a555000000000000000000", psbt.GetData().GetHex().c_str());
  Psbt psbt2(psbt.GetData());
  EXPECT_STREQ("cHNidP8BAJoCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8CAOH1BQAAAAAWABSzIr3c5jO4UaxzcKtFTws2egZU5QDh9QUAAAAAFgAUyrjFOm6PwCltHNORWjB9UcSRpVUAAAAAAAAAAAA=", psbt2.GetBase64().c_str());
}

TEST(Psbt, Join) {
  Psbt psbt_input_only("cHNidP8BAFwCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8AAAAAAAABAPYCAAAAAAEB8Zk/6OcYlULuRQYljhcCAb4pJwPNJ1rLCezhZnL9hIsAAAAAFxYAFKye+AsnrxydlcHbXXYTGTIrxC/F/////wIIBBAkAQAAABYAFAneKgQxy7NET8IsrZ2aD9CWOXIQAOH1BQAAAAAXqRRQn1mF9OkKFPuQ45MW/bTzrJdTB4cCRzBEAiAeB99yHDMiQZ6PNtB+6uR5WXW6DZ0ZYwyjzT3A1JZxcgIgFUKOe+BrZWdQFTkFC9eRo4DwC73bxQl+qXunvkAXEUoBIQJK70Ox1ax7pQFJmNY86sWDlZ0f3GbqJpnNhO6vgqKDBgAAAAABASAA4fUFAAAAABepFFCfWYX06QoU+5Djkxb9tPOsl1MHhwEDBAEAAAABBBYAFJYsTgjzNtOvvDQVydNZrhBARwUgIgYCVlJIRgs8GG3s8T2wbwck/VC0fMPkicMAdsg2PVA4zv4YKnBHYCwAAIAAAACAAAAAgAEAAAABAAAAAAEAvwIAAAABxtLqNuLoArUt2sZl2svtL4MbUmNFnhynNPXJRddRXkAAAAAAakcwRAIgEblsfS0NLo3LNxOOGKzEYHUpZa3Zy4eH31w0nfDirmYCIC6TrzG2T1Fm5WBYGVVdq+xXvnlDAPrbNwUvMd3eqZBcASED49JEo5Z+C4d2X9qGxf84iF90mTlTuVhDiK7zCyavauz/////AXhRzR0AAAAAGXapFI0gRDqRlp47yg4kDND/5NyYxj3iiKwAAAAAIgYC2faIjyhaFaahiAoiAsKyMKQtd89i2mpIrKQZgmLNo8cYnWtthiwAAIAAAACAAAAAgAAAAAABAAAAAA==");
  Psbt psbt_output_only("cHNidP8BAEgCAAAAAAIA4fUFAAAAABYAFLMivdzmM7hRrHNwq0VPCzZ6BlTlAOH1BQAAAAAWABTKuMU6bo/AKW0c05FaMH1RxJGlVQAAAAAAIgIDRzv8jHcMGyIKLnquS632wNfq8pAo1bKdNDgBK7KJ74EYKnBHYCwAAIAAAACAAAAAgAAAAAACAAAAACICA2R0r/JjPDUYZVOftStiudb7nk4jV2Yo4fCgp5k0WOBsGJ1rbYYsAACAAAAAgAAAAIAAAAAAAgAAAAA=");
  Psbt psbt;

  psbt = psbt_input_only;
  psbt.Join(psbt_output_only);
  EXPECT_STREQ("70736274ff01009a0200000002267ffd76eae6b6c13ffb86ceabaa24d42485520233c2c3805e0ad764709578c00100000000ffffffff7d25462a42ff2cb1ba5401e5893259db3ca4bcf5cf6ab9715d35163d31c9ecc00000000000ffffffff0200e1f50500000000160014b322bddce633b851ac7370ab454f0b367a0654e500e1f50500000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a55500000000000100f602000000000101f1993fe8e7189542ee4506258e170201be292703cd275acb09ece16672fd848b0000000017160014ac9ef80b27af1c9d95c1db5d761319322bc42fc5ffffffff02080410240100000016001409de2a0431cbb3444fc22cad9d9a0fd09639721000e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac975307870247304402201e07df721c3322419e8f36d07eeae4795975ba0d9d19630ca3cd3dc0d4967172022015428e7be06b6567501539050bd791a380f00bbddbc5097ea97ba7be4017114a0121024aef43b1d5ac7ba5014998d63ceac583959d1fdc66ea2699cd84eeaf82a283060000000001012000e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac97530787010304010000000104160014962c4e08f336d3afbc3415c9d359ae1040470520220602565248460b3c186decf13db06f0724fd50b47cc3e489c30076c8363d5038cefe182a7047602c00008000000080000000800100000001000000000100bf0200000001c6d2ea36e2e802b52ddac665dacbed2f831b5263459e1ca734f5c945d7515e40000000006a473044022011b96c7d2d0d2e8dcb37138e18acc460752965add9cb8787df5c349df0e2ae6602202e93af31b64f5166e5605819555dabec57be794300fadb37052f31dddea9905c012103e3d244a3967e0b87765fda86c5ff38885f74993953b9584388aef30b26af6aecffffffff017851cd1d000000001976a9148d20443a91969e3bca0e240cd0ffe4dc98c63de288ac00000000220602d9f6888f285a15a6a1880a2202c2b230a42d77cf62da6a48aca4198262cda3c7189d6b6d862c0000800000008000000080000000000100000000220203473bfc8c770c1b220a2e7aae4badf6c0d7eaf29028d5b29d3438012bb289ef81182a7047602c00008000000080000000800000000002000000002202036474aff2633c351865539fb52b62b9d6fb9e4e23576628e1f0a0a7993458e06c189d6b6d862c0000800000008000000080000000000200000000", psbt.GetData().GetHex().c_str());
  Psbt psbt2(psbt.GetData());
  EXPECT_STREQ("cHNidP8BAJoCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8CAOH1BQAAAAAWABSzIr3c5jO4UaxzcKtFTws2egZU5QDh9QUAAAAAFgAUyrjFOm6PwCltHNORWjB9UcSRpVUAAAAAAAEA9gIAAAAAAQHxmT/o5xiVQu5FBiWOFwIBviknA80nWssJ7OFmcv2EiwAAAAAXFgAUrJ74CyevHJ2VwdtddhMZMivEL8X/////AggEECQBAAAAFgAUCd4qBDHLs0RPwiytnZoP0JY5chAA4fUFAAAAABepFFCfWYX06QoU+5Djkxb9tPOsl1MHhwJHMEQCIB4H33IcMyJBno820H7q5HlZdboNnRljDKPNPcDUlnFyAiAVQo574GtlZ1AVOQUL15GjgPALvdvFCX6pe6e+QBcRSgEhAkrvQ7HVrHulAUmY1jzqxYOVnR/cZuommc2E7q+CooMGAAAAAAEBIADh9QUAAAAAF6kUUJ9ZhfTpChT7kOOTFv2086yXUweHAQMEAQAAAAEEFgAUlixOCPM206+8NBXJ01muEEBHBSAiBgJWUkhGCzwYbezxPbBvByT9ULR8w+SJwwB2yDY9UDjO/hgqcEdgLAAAgAAAAIAAAACAAQAAAAEAAAAAAQC/AgAAAAHG0uo24ugCtS3axmXay+0vgxtSY0WeHKc09clF11FeQAAAAABqRzBEAiARuWx9LQ0ujcs3E44YrMRgdSllrdnLh4ffXDSd8OKuZgIgLpOvMbZPUWblYFgZVV2r7Fe+eUMA+ts3BS8x3d6pkFwBIQPj0kSjln4Lh3Zf2obF/ziIX3SZOVO5WEOIrvMLJq9q7P////8BeFHNHQAAAAAZdqkUjSBEOpGWnjvKDiQM0P/k3JjGPeKIrAAAAAAiBgLZ9oiPKFoVpqGICiICwrIwpC13z2LaakispBmCYs2jxxida22GLAAAgAAAAIAAAACAAAAAAAEAAAAAIgIDRzv8jHcMGyIKLnquS632wNfq8pAo1bKdNDgBK7KJ74EYKnBHYCwAAIAAAACAAAAAgAAAAAACAAAAACICA2R0r/JjPDUYZVOftStiudb7nk4jV2Yo4fCgp5k0WOBsGJ1rbYYsAACAAAAAgAAAAIAAAAAAAgAAAAA=", psbt2.GetBase64().c_str());
}

TEST(Psbt, JoinWithInput) {
  Psbt psbt_input_only("cHNidP8BAFwCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8AAAAAAAABAPYCAAAAAAEB8Zk/6OcYlULuRQYljhcCAb4pJwPNJ1rLCezhZnL9hIsAAAAAFxYAFKye+AsnrxydlcHbXXYTGTIrxC/F/////wIIBBAkAQAAABYAFAneKgQxy7NET8IsrZ2aD9CWOXIQAOH1BQAAAAAXqRRQn1mF9OkKFPuQ45MW/bTzrJdTB4cCRzBEAiAeB99yHDMiQZ6PNtB+6uR5WXW6DZ0ZYwyjzT3A1JZxcgIgFUKOe+BrZWdQFTkFC9eRo4DwC73bxQl+qXunvkAXEUoBIQJK70Ox1ax7pQFJmNY86sWDlZ0f3GbqJpnNhO6vgqKDBgAAAAABASAA4fUFAAAAABepFFCfWYX06QoU+5Djkxb9tPOsl1MHhwEDBAEAAAABBBYAFJYsTgjzNtOvvDQVydNZrhBARwUgIgYCVlJIRgs8GG3s8T2wbwck/VC0fMPkicMAdsg2PVA4zv4YKnBHYCwAAIAAAACAAAAAgAEAAAABAAAAAAEAvwIAAAABxtLqNuLoArUt2sZl2svtL4MbUmNFnhynNPXJRddRXkAAAAAAakcwRAIgEblsfS0NLo3LNxOOGKzEYHUpZa3Zy4eH31w0nfDirmYCIC6TrzG2T1Fm5WBYGVVdq+xXvnlDAPrbNwUvMd3eqZBcASED49JEo5Z+C4d2X9qGxf84iF90mTlTuVhDiK7zCyavauz/////AXhRzR0AAAAAGXapFI0gRDqRlp47yg4kDND/5NyYxj3iiKwAAAAAIgYC2faIjyhaFaahiAoiAsKyMKQtd89i2mpIrKQZgmLNo8cYnWtthiwAAIAAAACAAAAAgAAAAAABAAAAAA==");
  Psbt psbt_output_only("cHNidP8BAEgCAAAAAAIA4fUFAAAAABYAFLMivdzmM7hRrHNwq0VPCzZ6BlTlAOH1BQAAAAAWABTKuMU6bo/AKW0c05FaMH1RxJGlVQAAAAAAIgIDRzv8jHcMGyIKLnquS632wNfq8pAo1bKdNDgBK7KJ74EYKnBHYCwAAIAAAACAAAAAgAAAAAACAAAAACICA2R0r/JjPDUYZVOftStiudb7nk4jV2Yo4fCgp5k0WOBsGJ1rbYYsAACAAAAAgAAAAIAAAAAAAgAAAAA=");
  Psbt psbt;

  psbt = psbt_output_only;
  psbt.Join(psbt_input_only);
  EXPECT_STREQ("70736274ff01009a0200000002267ffd76eae6b6c13ffb86ceabaa24d42485520233c2c3805e0ad764709578c00100000000ffffffff7d25462a42ff2cb1ba5401e5893259db3ca4bcf5cf6ab9715d35163d31c9ecc00000000000ffffffff0200e1f50500000000160014b322bddce633b851ac7370ab454f0b367a0654e500e1f50500000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a55500000000000100f602000000000101f1993fe8e7189542ee4506258e170201be292703cd275acb09ece16672fd848b0000000017160014ac9ef80b27af1c9d95c1db5d761319322bc42fc5ffffffff02080410240100000016001409de2a0431cbb3444fc22cad9d9a0fd09639721000e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac975307870247304402201e07df721c3322419e8f36d07eeae4795975ba0d9d19630ca3cd3dc0d4967172022015428e7be06b6567501539050bd791a380f00bbddbc5097ea97ba7be4017114a0121024aef43b1d5ac7ba5014998d63ceac583959d1fdc66ea2699cd84eeaf82a283060000000001012000e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac97530787010304010000000104160014962c4e08f336d3afbc3415c9d359ae1040470520220602565248460b3c186decf13db06f0724fd50b47cc3e489c30076c8363d5038cefe182a7047602c00008000000080000000800100000001000000000100bf0200000001c6d2ea36e2e802b52ddac665dacbed2f831b5263459e1ca734f5c945d7515e40000000006a473044022011b96c7d2d0d2e8dcb37138e18acc460752965add9cb8787df5c349df0e2ae6602202e93af31b64f5166e5605819555dabec57be794300fadb37052f31dddea9905c012103e3d244a3967e0b87765fda86c5ff38885f74993953b9584388aef30b26af6aecffffffff017851cd1d000000001976a9148d20443a91969e3bca0e240cd0ffe4dc98c63de288ac00000000220602d9f6888f285a15a6a1880a2202c2b230a42d77cf62da6a48aca4198262cda3c7189d6b6d862c0000800000008000000080000000000100000000220203473bfc8c770c1b220a2e7aae4badf6c0d7eaf29028d5b29d3438012bb289ef81182a7047602c00008000000080000000800000000002000000002202036474aff2633c351865539fb52b62b9d6fb9e4e23576628e1f0a0a7993458e06c189d6b6d862c0000800000008000000080000000000200000000", psbt.GetData().GetHex().c_str());
  Psbt psbt2(psbt.GetData());
  EXPECT_STREQ("cHNidP8BAJoCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8CAOH1BQAAAAAWABSzIr3c5jO4UaxzcKtFTws2egZU5QDh9QUAAAAAFgAUyrjFOm6PwCltHNORWjB9UcSRpVUAAAAAAAEA9gIAAAAAAQHxmT/o5xiVQu5FBiWOFwIBviknA80nWssJ7OFmcv2EiwAAAAAXFgAUrJ74CyevHJ2VwdtddhMZMivEL8X/////AggEECQBAAAAFgAUCd4qBDHLs0RPwiytnZoP0JY5chAA4fUFAAAAABepFFCfWYX06QoU+5Djkxb9tPOsl1MHhwJHMEQCIB4H33IcMyJBno820H7q5HlZdboNnRljDKPNPcDUlnFyAiAVQo574GtlZ1AVOQUL15GjgPALvdvFCX6pe6e+QBcRSgEhAkrvQ7HVrHulAUmY1jzqxYOVnR/cZuommc2E7q+CooMGAAAAAAEBIADh9QUAAAAAF6kUUJ9ZhfTpChT7kOOTFv2086yXUweHAQMEAQAAAAEEFgAUlixOCPM206+8NBXJ01muEEBHBSAiBgJWUkhGCzwYbezxPbBvByT9ULR8w+SJwwB2yDY9UDjO/hgqcEdgLAAAgAAAAIAAAACAAQAAAAEAAAAAAQC/AgAAAAHG0uo24ugCtS3axmXay+0vgxtSY0WeHKc09clF11FeQAAAAABqRzBEAiARuWx9LQ0ujcs3E44YrMRgdSllrdnLh4ffXDSd8OKuZgIgLpOvMbZPUWblYFgZVV2r7Fe+eUMA+ts3BS8x3d6pkFwBIQPj0kSjln4Lh3Zf2obF/ziIX3SZOVO5WEOIrvMLJq9q7P////8BeFHNHQAAAAAZdqkUjSBEOpGWnjvKDiQM0P/k3JjGPeKIrAAAAAAiBgLZ9oiPKFoVpqGICiICwrIwpC13z2LaakispBmCYs2jxxida22GLAAAgAAAAIAAAACAAAAAAAEAAAAAIgIDRzv8jHcMGyIKLnquS632wNfq8pAo1bKdNDgBK7KJ74EYKnBHYCwAAIAAAACAAAAAgAAAAAACAAAAACICA2R0r/JjPDUYZVOftStiudb7nk4jV2Yo4fCgp5k0WOBsGJ1rbYYsAACAAAAAgAAAAIAAAAAAAgAAAAA=", psbt2.GetBase64().c_str());
}

TEST(Psbt, JoinDuplicateInput) {
  Psbt psbt_input_only("cHNidP8BAFwCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8AAAAAAAABAPYCAAAAAAEB8Zk/6OcYlULuRQYljhcCAb4pJwPNJ1rLCezhZnL9hIsAAAAAFxYAFKye+AsnrxydlcHbXXYTGTIrxC/F/////wIIBBAkAQAAABYAFAneKgQxy7NET8IsrZ2aD9CWOXIQAOH1BQAAAAAXqRRQn1mF9OkKFPuQ45MW/bTzrJdTB4cCRzBEAiAeB99yHDMiQZ6PNtB+6uR5WXW6DZ0ZYwyjzT3A1JZxcgIgFUKOe+BrZWdQFTkFC9eRo4DwC73bxQl+qXunvkAXEUoBIQJK70Ox1ax7pQFJmNY86sWDlZ0f3GbqJpnNhO6vgqKDBgAAAAABASAA4fUFAAAAABepFFCfWYX06QoU+5Djkxb9tPOsl1MHhwEDBAEAAAABBBYAFJYsTgjzNtOvvDQVydNZrhBARwUgIgYCVlJIRgs8GG3s8T2wbwck/VC0fMPkicMAdsg2PVA4zv4YKnBHYCwAAIAAAACAAAAAgAEAAAABAAAAAAEAvwIAAAABxtLqNuLoArUt2sZl2svtL4MbUmNFnhynNPXJRddRXkAAAAAAakcwRAIgEblsfS0NLo3LNxOOGKzEYHUpZa3Zy4eH31w0nfDirmYCIC6TrzG2T1Fm5WBYGVVdq+xXvnlDAPrbNwUvMd3eqZBcASED49JEo5Z+C4d2X9qGxf84iF90mTlTuVhDiK7zCyavauz/////AXhRzR0AAAAAGXapFI0gRDqRlp47yg4kDND/5NyYxj3iiKwAAAAAIgYC2faIjyhaFaahiAoiAsKyMKQtd89i2mpIrKQZgmLNo8cYnWtthiwAAIAAAACAAAAAgAAAAAABAAAAAA==");
  Psbt psbt("cHNidP8BAJoCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8CAOH1BQAAAAAWABSzIr3c5jO4UaxzcKtFTws2egZU5QDh9QUAAAAAFgAUyrjFOm6PwCltHNORWjB9UcSRpVUAAAAAAAEA9gIAAAAAAQHxmT/o5xiVQu5FBiWOFwIBviknA80nWssJ7OFmcv2EiwAAAAAXFgAUrJ74CyevHJ2VwdtddhMZMivEL8X/////AggEECQBAAAAFgAUCd4qBDHLs0RPwiytnZoP0JY5chAA4fUFAAAAABepFFCfWYX06QoU+5Djkxb9tPOsl1MHhwJHMEQCIB4H33IcMyJBno820H7q5HlZdboNnRljDKPNPcDUlnFyAiAVQo574GtlZ1AVOQUL15GjgPALvdvFCX6pe6e+QBcRSgEhAkrvQ7HVrHulAUmY1jzqxYOVnR/cZuommc2E7q+CooMGAAAAAAEBIADh9QUAAAAAF6kUUJ9ZhfTpChT7kOOTFv2086yXUweHAQMEAQAAAAEEFgAUlixOCPM206+8NBXJ01muEEBHBSAiBgJWUkhGCzwYbezxPbBvByT9ULR8w+SJwwB2yDY9UDjO/hgqcEdgLAAAgAAAAIAAAACAAQAAAAEAAAAAAQC/AgAAAAHG0uo24ugCtS3axmXay+0vgxtSY0WeHKc09clF11FeQAAAAABqRzBEAiARuWx9LQ0ujcs3E44YrMRgdSllrdnLh4ffXDSd8OKuZgIgLpOvMbZPUWblYFgZVV2r7Fe+eUMA+ts3BS8x3d6pkFwBIQPj0kSjln4Lh3Zf2obF/ziIX3SZOVO5WEOIrvMLJq9q7P////8BeFHNHQAAAAAZdqkUjSBEOpGWnjvKDiQM0P/k3JjGPeKIrAAAAAAiBgLZ9oiPKFoVpqGICiICwrIwpC13z2LaakispBmCYs2jxxida22GLAAAgAAAAIAAAACAAAAAAAEAAAAAIgIDRzv8jHcMGyIKLnquS632wNfq8pAo1bKdNDgBK7KJ74EYKnBHYCwAAIAAAACAAAAAgAAAAAACAAAAACICA2R0r/JjPDUYZVOftStiudb7nk4jV2Yo4fCgp5k0WOBsGJ1rbYYsAACAAAAAgAAAAIAAAAAAAgAAAAA=");

  try {
    psbt.Join(psbt_input_only, true);
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
  }

  EXPECT_STREQ("70736274ff01009a0200000002267ffd76eae6b6c13ffb86ceabaa24d42485520233c2c3805e0ad764709578c00100000000ffffffff7d25462a42ff2cb1ba5401e5893259db3ca4bcf5cf6ab9715d35163d31c9ecc00000000000ffffffff0200e1f50500000000160014b322bddce633b851ac7370ab454f0b367a0654e500e1f50500000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a55500000000000100f602000000000101f1993fe8e7189542ee4506258e170201be292703cd275acb09ece16672fd848b0000000017160014ac9ef80b27af1c9d95c1db5d761319322bc42fc5ffffffff02080410240100000016001409de2a0431cbb3444fc22cad9d9a0fd09639721000e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac975307870247304402201e07df721c3322419e8f36d07eeae4795975ba0d9d19630ca3cd3dc0d4967172022015428e7be06b6567501539050bd791a380f00bbddbc5097ea97ba7be4017114a0121024aef43b1d5ac7ba5014998d63ceac583959d1fdc66ea2699cd84eeaf82a283060000000001012000e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac97530787010304010000000104160014962c4e08f336d3afbc3415c9d359ae1040470520220602565248460b3c186decf13db06f0724fd50b47cc3e489c30076c8363d5038cefe182a7047602c00008000000080000000800100000001000000000100bf0200000001c6d2ea36e2e802b52ddac665dacbed2f831b5263459e1ca734f5c945d7515e40000000006a473044022011b96c7d2d0d2e8dcb37138e18acc460752965add9cb8787df5c349df0e2ae6602202e93af31b64f5166e5605819555dabec57be794300fadb37052f31dddea9905c012103e3d244a3967e0b87765fda86c5ff38885f74993953b9584388aef30b26af6aecffffffff017851cd1d000000001976a9148d20443a91969e3bca0e240cd0ffe4dc98c63de288ac00000000220602d9f6888f285a15a6a1880a2202c2b230a42d77cf62da6a48aca4198262cda3c7189d6b6d862c0000800000008000000080000000000100000000220203473bfc8c770c1b220a2e7aae4badf6c0d7eaf29028d5b29d3438012bb289ef81182a7047602c00008000000080000000800000000002000000002202036474aff2633c351865539fb52b62b9d6fb9e4e23576628e1f0a0a7993458e06c189d6b6d862c0000800000008000000080000000000200000000", psbt.GetData().GetHex().c_str());
  Psbt psbt2(psbt.GetData());
  EXPECT_STREQ("cHNidP8BAJoCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8CAOH1BQAAAAAWABSzIr3c5jO4UaxzcKtFTws2egZU5QDh9QUAAAAAFgAUyrjFOm6PwCltHNORWjB9UcSRpVUAAAAAAAEA9gIAAAAAAQHxmT/o5xiVQu5FBiWOFwIBviknA80nWssJ7OFmcv2EiwAAAAAXFgAUrJ74CyevHJ2VwdtddhMZMivEL8X/////AggEECQBAAAAFgAUCd4qBDHLs0RPwiytnZoP0JY5chAA4fUFAAAAABepFFCfWYX06QoU+5Djkxb9tPOsl1MHhwJHMEQCIB4H33IcMyJBno820H7q5HlZdboNnRljDKPNPcDUlnFyAiAVQo574GtlZ1AVOQUL15GjgPALvdvFCX6pe6e+QBcRSgEhAkrvQ7HVrHulAUmY1jzqxYOVnR/cZuommc2E7q+CooMGAAAAAAEBIADh9QUAAAAAF6kUUJ9ZhfTpChT7kOOTFv2086yXUweHAQMEAQAAAAEEFgAUlixOCPM206+8NBXJ01muEEBHBSAiBgJWUkhGCzwYbezxPbBvByT9ULR8w+SJwwB2yDY9UDjO/hgqcEdgLAAAgAAAAIAAAACAAQAAAAEAAAAAAQC/AgAAAAHG0uo24ugCtS3axmXay+0vgxtSY0WeHKc09clF11FeQAAAAABqRzBEAiARuWx9LQ0ujcs3E44YrMRgdSllrdnLh4ffXDSd8OKuZgIgLpOvMbZPUWblYFgZVV2r7Fe+eUMA+ts3BS8x3d6pkFwBIQPj0kSjln4Lh3Zf2obF/ziIX3SZOVO5WEOIrvMLJq9q7P////8BeFHNHQAAAAAZdqkUjSBEOpGWnjvKDiQM0P/k3JjGPeKIrAAAAAAiBgLZ9oiPKFoVpqGICiICwrIwpC13z2LaakispBmCYs2jxxida22GLAAAgAAAAIAAAACAAAAAAAEAAAAAIgIDRzv8jHcMGyIKLnquS632wNfq8pAo1bKdNDgBK7KJ74EYKnBHYCwAAIAAAACAAAAAgAAAAAACAAAAACICA2R0r/JjPDUYZVOftStiudb7nk4jV2Yo4fCgp5k0WOBsGJ1rbYYsAACAAAAAgAAAAIAAAAAAAgAAAAA=", psbt2.GetBase64().c_str());
}

TEST(Psbt, UsecaseTest1) {
  static const std::string seed1 = "8bc106907003ea0b55f3ed4ce2fcf9a198d8c43f07e6ade8aacc5c20c33db12e";
  static const std::string seed2 = "d3e3539eafb6af1f0ae374ecffd33bed394f5eb2e39f8957be63c258ac32ca97";

  HDWallet wallet1 = HDWallet(ByteData(g_psbt_seed1));
  HDWallet wallet2 = HDWallet(ByteData(g_psbt_seed2));
  std::string path1 = "44h/0h/0h/1/1";
  std::string path2 = "44h/0h/0h/0/1";
  auto key_in1 = wallet1.GeneratePrivkeyData(NetType::kTestnet, path1);
  auto key_in2 = wallet2.GeneratePrivkeyData(NetType::kTestnet, path2);

  // Creator,Updater
  Psbt psbt("cHNidP8BAJoCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8CAOH1BQAAAAAWABSzIr3c5jO4UaxzcKtFTws2egZU5QDh9QUAAAAAFgAUyrjFOm6PwCltHNORWjB9UcSRpVUAAAAAAAEA9gIAAAAAAQHxmT/o5xiVQu5FBiWOFwIBviknA80nWssJ7OFmcv2EiwAAAAAXFgAUrJ74CyevHJ2VwdtddhMZMivEL8X/////AggEECQBAAAAFgAUCd4qBDHLs0RPwiytnZoP0JY5chAA4fUFAAAAABepFFCfWYX06QoU+5Djkxb9tPOsl1MHhwJHMEQCIB4H33IcMyJBno820H7q5HlZdboNnRljDKPNPcDUlnFyAiAVQo574GtlZ1AVOQUL15GjgPALvdvFCX6pe6e+QBcRSgEhAkrvQ7HVrHulAUmY1jzqxYOVnR/cZuommc2E7q+CooMGAAAAAAEBIADh9QUAAAAAF6kUUJ9ZhfTpChT7kOOTFv2086yXUweHAQMEAQAAAAEEFgAUlixOCPM206+8NBXJ01muEEBHBSAiBgJWUkhGCzwYbezxPbBvByT9ULR8w+SJwwB2yDY9UDjO/hgqcEdgLAAAgAAAAIAAAACAAQAAAAEAAAAAAQC/AgAAAAHG0uo24ugCtS3axmXay+0vgxtSY0WeHKc09clF11FeQAAAAABqRzBEAiARuWx9LQ0ujcs3E44YrMRgdSllrdnLh4ffXDSd8OKuZgIgLpOvMbZPUWblYFgZVV2r7Fe+eUMA+ts3BS8x3d6pkFwBIQPj0kSjln4Lh3Zf2obF/ziIX3SZOVO5WEOIrvMLJq9q7P////8BeFHNHQAAAAAZdqkUjSBEOpGWnjvKDiQM0P/k3JjGPeKIrAAAAAAiBgLZ9oiPKFoVpqGICiICwrIwpC13z2LaakispBmCYs2jxxida22GLAAAgAAAAIAAAACAAAAAAAEAAAAAIgIDRzv8jHcMGyIKLnquS632wNfq8pAo1bKdNDgBK7KJ74EYKnBHYCwAAIAAAACAAAAAgAAAAAACAAAAACICA2R0r/JjPDUYZVOftStiudb7nk4jV2Yo4fCgp5k0WOBsGJ1rbYYsAACAAAAAgAAAAIAAAAAAAgAAAAA=");

  // Signer
  Psbt psbt1;
  Psbt psbt2;
  try {
    EXPECT_STREQ("cMjw7gbCpT75qh8s5ztdkXBJ1B1C82U6ohC3D2eQgqD9PnJWar9G", key_in1.GetPrivkey().GetWif().c_str());
    psbt1 = psbt;
    psbt1.Sign(key_in1.GetPrivkey());
    EXPECT_STREQ("cHNidP8BAJoCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8CAOH1BQAAAAAWABSzIr3c5jO4UaxzcKtFTws2egZU5QDh9QUAAAAAFgAUyrjFOm6PwCltHNORWjB9UcSRpVUAAAAAAAEA9gIAAAAAAQHxmT/o5xiVQu5FBiWOFwIBviknA80nWssJ7OFmcv2EiwAAAAAXFgAUrJ74CyevHJ2VwdtddhMZMivEL8X/////AggEECQBAAAAFgAUCd4qBDHLs0RPwiytnZoP0JY5chAA4fUFAAAAABepFFCfWYX06QoU+5Djkxb9tPOsl1MHhwJHMEQCIB4H33IcMyJBno820H7q5HlZdboNnRljDKPNPcDUlnFyAiAVQo574GtlZ1AVOQUL15GjgPALvdvFCX6pe6e+QBcRSgEhAkrvQ7HVrHulAUmY1jzqxYOVnR/cZuommc2E7q+CooMGAAAAAAEBIADh9QUAAAAAF6kUUJ9ZhfTpChT7kOOTFv2086yXUweHIgICVlJIRgs8GG3s8T2wbwck/VC0fMPkicMAdsg2PVA4zv5HMEQCICmYYqZ6m0VNbNpf7jSlLZCJv6AkRE2IAxw0qY4pi9vKAiBLR/z1B7gJVBCOA3VnP9vdCExfu5lPbJUXIPLrL2u4ZgEBAwQBAAAAAQQWABSWLE4I8zbTr7w0FcnTWa4QQEcFICIGAlZSSEYLPBht7PE9sG8HJP1QtHzD5InDAHbINj1QOM7+GCpwR2AsAACAAAAAgAAAAIABAAAAAQAAAAABAL8CAAAAAcbS6jbi6AK1LdrGZdrL7S+DG1JjRZ4cpzT1yUXXUV5AAAAAAGpHMEQCIBG5bH0tDS6NyzcTjhisxGB1KWWt2cuHh99cNJ3w4q5mAiAuk68xtk9RZuVgWBlVXavsV755QwD62zcFLzHd3qmQXAEhA+PSRKOWfguHdl/ahsX/OIhfdJk5U7lYQ4iu8wsmr2rs/////wF4Uc0dAAAAABl2qRSNIEQ6kZaeO8oOJAzQ/+TcmMY94oisAAAAACIGAtn2iI8oWhWmoYgKIgLCsjCkLXfPYtpqSKykGYJizaPHGJ1rbYYsAACAAAAAgAAAAIAAAAAAAQAAAAAiAgNHO/yMdwwbIgoueq5LrfbA1+rykCjVsp00OAErsonvgRgqcEdgLAAAgAAAAIAAAACAAAAAAAIAAAAAIgIDZHSv8mM8NRhlU5+1K2K51vueTiNXZijh8KCnmTRY4GwYnWtthiwAAIAAAACAAAAAgAAAAAACAAAAAA==", psbt1.GetBase64().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }
  try {
    EXPECT_STREQ("cPvrDmg2L8vxM2usGWnV8Dq8otFuZ5ZQjEu2WkVnqJDABuKjdhrG", key_in2.GetPrivkey().GetWif().c_str());
    psbt2 = psbt;
    psbt2.Sign(key_in2.GetPrivkey());
    EXPECT_STREQ("cHNidP8BAJoCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8CAOH1BQAAAAAWABSzIr3c5jO4UaxzcKtFTws2egZU5QDh9QUAAAAAFgAUyrjFOm6PwCltHNORWjB9UcSRpVUAAAAAAAEA9gIAAAAAAQHxmT/o5xiVQu5FBiWOFwIBviknA80nWssJ7OFmcv2EiwAAAAAXFgAUrJ74CyevHJ2VwdtddhMZMivEL8X/////AggEECQBAAAAFgAUCd4qBDHLs0RPwiytnZoP0JY5chAA4fUFAAAAABepFFCfWYX06QoU+5Djkxb9tPOsl1MHhwJHMEQCIB4H33IcMyJBno820H7q5HlZdboNnRljDKPNPcDUlnFyAiAVQo574GtlZ1AVOQUL15GjgPALvdvFCX6pe6e+QBcRSgEhAkrvQ7HVrHulAUmY1jzqxYOVnR/cZuommc2E7q+CooMGAAAAAAEBIADh9QUAAAAAF6kUUJ9ZhfTpChT7kOOTFv2086yXUweHAQMEAQAAAAEEFgAUlixOCPM206+8NBXJ01muEEBHBSAiBgJWUkhGCzwYbezxPbBvByT9ULR8w+SJwwB2yDY9UDjO/hgqcEdgLAAAgAAAAIAAAACAAQAAAAEAAAAAAQC/AgAAAAHG0uo24ugCtS3axmXay+0vgxtSY0WeHKc09clF11FeQAAAAABqRzBEAiARuWx9LQ0ujcs3E44YrMRgdSllrdnLh4ffXDSd8OKuZgIgLpOvMbZPUWblYFgZVV2r7Fe+eUMA+ts3BS8x3d6pkFwBIQPj0kSjln4Lh3Zf2obF/ziIX3SZOVO5WEOIrvMLJq9q7P////8BeFHNHQAAAAAZdqkUjSBEOpGWnjvKDiQM0P/k3JjGPeKIrAAAAAAiAgLZ9oiPKFoVpqGICiICwrIwpC13z2LaakispBmCYs2jx0cwRAIgZuyVbpZ4PNYSLJwHcyo66ZMbaava/x82LCjtX6lnKNMCIGoW1K5G/tSNPQSSc3YTvZtR/j4kU7REbkR8Zb8sln4QASIGAtn2iI8oWhWmoYgKIgLCsjCkLXfPYtpqSKykGYJizaPHGJ1rbYYsAACAAAAAgAAAAIAAAAAAAQAAAAAiAgNHO/yMdwwbIgoueq5LrfbA1+rykCjVsp00OAErsonvgRgqcEdgLAAAgAAAAIAAAACAAAAAAAIAAAAAIgIDZHSv8mM8NRhlU5+1K2K51vueTiNXZijh8KCnmTRY4GwYnWtthiwAAIAAAACAAAAAgAAAAAACAAAAAA==", psbt2.GetBase64().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  // Combiner
  try {
    psbt.Combine(psbt1);
    psbt.Combine(psbt2);
    EXPECT_STREQ("70736274ff01009a0200000002267ffd76eae6b6c13ffb86ceabaa24d42485520233c2c3805e0ad764709578c00100000000ffffffff7d25462a42ff2cb1ba5401e5893259db3ca4bcf5cf6ab9715d35163d31c9ecc00000000000ffffffff0200e1f50500000000160014b322bddce633b851ac7370ab454f0b367a0654e500e1f50500000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a55500000000000100f602000000000101f1993fe8e7189542ee4506258e170201be292703cd275acb09ece16672fd848b0000000017160014ac9ef80b27af1c9d95c1db5d761319322bc42fc5ffffffff02080410240100000016001409de2a0431cbb3444fc22cad9d9a0fd09639721000e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac975307870247304402201e07df721c3322419e8f36d07eeae4795975ba0d9d19630ca3cd3dc0d4967172022015428e7be06b6567501539050bd791a380f00bbddbc5097ea97ba7be4017114a0121024aef43b1d5ac7ba5014998d63ceac583959d1fdc66ea2699cd84eeaf82a283060000000001012000e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac97530787220202565248460b3c186decf13db06f0724fd50b47cc3e489c30076c8363d5038cefe4730440220299862a67a9b454d6cda5fee34a52d9089bfa024444d88031c34a98e298bdbca02204b47fcf507b80954108e0375673fdbdd084c5fbb994f6c951720f2eb2f6bb86601010304010000000104160014962c4e08f336d3afbc3415c9d359ae1040470520220602565248460b3c186decf13db06f0724fd50b47cc3e489c30076c8363d5038cefe182a7047602c00008000000080000000800100000001000000000100bf0200000001c6d2ea36e2e802b52ddac665dacbed2f831b5263459e1ca734f5c945d7515e40000000006a473044022011b96c7d2d0d2e8dcb37138e18acc460752965add9cb8787df5c349df0e2ae6602202e93af31b64f5166e5605819555dabec57be794300fadb37052f31dddea9905c012103e3d244a3967e0b87765fda86c5ff38885f74993953b9584388aef30b26af6aecffffffff017851cd1d000000001976a9148d20443a91969e3bca0e240cd0ffe4dc98c63de288ac00000000220202d9f6888f285a15a6a1880a2202c2b230a42d77cf62da6a48aca4198262cda3c7473044022066ec956e96783cd6122c9c07732a3ae9931b69abdaff1f362c28ed5fa96728d302206a16d4ae46fed48d3d0492737613bd9b51fe3e2453b4446e447c65bf2c967e1001220602d9f6888f285a15a6a1880a2202c2b230a42d77cf62da6a48aca4198262cda3c7189d6b6d862c0000800000008000000080000000000100000000220203473bfc8c770c1b220a2e7aae4badf6c0d7eaf29028d5b29d3438012bb289ef81182a7047602c00008000000080000000800000000002000000002202036474aff2633c351865539fb52b62b9d6fb9e4e23576628e1f0a0a7993458e06c189d6b6d862c0000800000008000000080000000000200000000", psbt.GetData().GetHex().c_str());
    EXPECT_STREQ("cHNidP8BAJoCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8CAOH1BQAAAAAWABSzIr3c5jO4UaxzcKtFTws2egZU5QDh9QUAAAAAFgAUyrjFOm6PwCltHNORWjB9UcSRpVUAAAAAAAEA9gIAAAAAAQHxmT/o5xiVQu5FBiWOFwIBviknA80nWssJ7OFmcv2EiwAAAAAXFgAUrJ74CyevHJ2VwdtddhMZMivEL8X/////AggEECQBAAAAFgAUCd4qBDHLs0RPwiytnZoP0JY5chAA4fUFAAAAABepFFCfWYX06QoU+5Djkxb9tPOsl1MHhwJHMEQCIB4H33IcMyJBno820H7q5HlZdboNnRljDKPNPcDUlnFyAiAVQo574GtlZ1AVOQUL15GjgPALvdvFCX6pe6e+QBcRSgEhAkrvQ7HVrHulAUmY1jzqxYOVnR/cZuommc2E7q+CooMGAAAAAAEBIADh9QUAAAAAF6kUUJ9ZhfTpChT7kOOTFv2086yXUweHIgICVlJIRgs8GG3s8T2wbwck/VC0fMPkicMAdsg2PVA4zv5HMEQCICmYYqZ6m0VNbNpf7jSlLZCJv6AkRE2IAxw0qY4pi9vKAiBLR/z1B7gJVBCOA3VnP9vdCExfu5lPbJUXIPLrL2u4ZgEBAwQBAAAAAQQWABSWLE4I8zbTr7w0FcnTWa4QQEcFICIGAlZSSEYLPBht7PE9sG8HJP1QtHzD5InDAHbINj1QOM7+GCpwR2AsAACAAAAAgAAAAIABAAAAAQAAAAABAL8CAAAAAcbS6jbi6AK1LdrGZdrL7S+DG1JjRZ4cpzT1yUXXUV5AAAAAAGpHMEQCIBG5bH0tDS6NyzcTjhisxGB1KWWt2cuHh99cNJ3w4q5mAiAuk68xtk9RZuVgWBlVXavsV755QwD62zcFLzHd3qmQXAEhA+PSRKOWfguHdl/ahsX/OIhfdJk5U7lYQ4iu8wsmr2rs/////wF4Uc0dAAAAABl2qRSNIEQ6kZaeO8oOJAzQ/+TcmMY94oisAAAAACICAtn2iI8oWhWmoYgKIgLCsjCkLXfPYtpqSKykGYJizaPHRzBEAiBm7JVulng81hIsnAdzKjrpkxtpq9r/HzYsKO1fqWco0wIgahbUrkb+1I09BJJzdhO9m1H+PiRTtERuRHxlvyyWfhABIgYC2faIjyhaFaahiAoiAsKyMKQtd89i2mpIrKQZgmLNo8cYnWtthiwAAIAAAACAAAAAgAAAAAABAAAAACICA0c7/Ix3DBsiCi56rkut9sDX6vKQKNWynTQ4ASuyie+BGCpwR2AsAACAAAAAgAAAAIAAAAAAAgAAAAAiAgNkdK/yYzw1GGVTn7UrYrnW+55OI1dmKOHwoKeZNFjgbBida22GLAAAgAAAAIAAAACAAAAAAAIAAAAA", psbt.GetBase64().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  // Input Finalizer
  try {
    EXPECT_FALSE(psbt.IsFinalized());
    EXPECT_FALSE(psbt.IsFinalizedInput(0));
#if 0
    auto sig1 = psbt.GetTxInSignature(0, key_in1.GetPubkey());
    auto sig2 = psbt.GetTxInSignature(1, key_in2.GetPubkey());
    psbt.SetTxInFinalScript(
        0, std::vector<ByteData>{sig1, key_in1.GetPubkey().GetData()});
    EXPECT_FALSE(psbt.IsFinalizedInput(1));
    psbt.SetTxInFinalScript(
        1, std::vector<ByteData>{sig2, key_in2.GetPubkey().GetData()});
    psbt.ClearTxInSignData(0);
    psbt.ClearTxInSignData(1);
#else
    psbt.Finalize();
#endif
    EXPECT_TRUE(psbt.IsFinalizedInput(0));
    EXPECT_TRUE(psbt.IsFinalizedInput(1));
    EXPECT_STREQ("70736274ff01009a0200000002267ffd76eae6b6c13ffb86ceabaa24d42485520233c2c3805e0ad764709578c00100000000ffffffff7d25462a42ff2cb1ba5401e5893259db3ca4bcf5cf6ab9715d35163d31c9ecc00000000000ffffffff0200e1f50500000000160014b322bddce633b851ac7370ab454f0b367a0654e500e1f50500000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a55500000000000100f602000000000101f1993fe8e7189542ee4506258e170201be292703cd275acb09ece16672fd848b0000000017160014ac9ef80b27af1c9d95c1db5d761319322bc42fc5ffffffff02080410240100000016001409de2a0431cbb3444fc22cad9d9a0fd09639721000e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac975307870247304402201e07df721c3322419e8f36d07eeae4795975ba0d9d19630ca3cd3dc0d4967172022015428e7be06b6567501539050bd791a380f00bbddbc5097ea97ba7be4017114a0121024aef43b1d5ac7ba5014998d63ceac583959d1fdc66ea2699cd84eeaf82a283060000000001012000e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac97530787010717160014962c4e08f336d3afbc3415c9d359ae104047052001086b024730440220299862a67a9b454d6cda5fee34a52d9089bfa024444d88031c34a98e298bdbca02204b47fcf507b80954108e0375673fdbdd084c5fbb994f6c951720f2eb2f6bb866012102565248460b3c186decf13db06f0724fd50b47cc3e489c30076c8363d5038cefe000100bf0200000001c6d2ea36e2e802b52ddac665dacbed2f831b5263459e1ca734f5c945d7515e40000000006a473044022011b96c7d2d0d2e8dcb37138e18acc460752965add9cb8787df5c349df0e2ae6602202e93af31b64f5166e5605819555dabec57be794300fadb37052f31dddea9905c012103e3d244a3967e0b87765fda86c5ff38885f74993953b9584388aef30b26af6aecffffffff017851cd1d000000001976a9148d20443a91969e3bca0e240cd0ffe4dc98c63de288ac0000000001076a473044022066ec956e96783cd6122c9c07732a3ae9931b69abdaff1f362c28ed5fa96728d302206a16d4ae46fed48d3d0492737613bd9b51fe3e2453b4446e447c65bf2c967e10012102d9f6888f285a15a6a1880a2202c2b230a42d77cf62da6a48aca4198262cda3c700220203473bfc8c770c1b220a2e7aae4badf6c0d7eaf29028d5b29d3438012bb289ef81182a7047602c00008000000080000000800000000002000000002202036474aff2633c351865539fb52b62b9d6fb9e4e23576628e1f0a0a7993458e06c189d6b6d862c0000800000008000000080000000000200000000", psbt.GetData().GetHex().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  // Transaction Extractor
  try {
    EXPECT_TRUE(psbt.IsFinalized());
    auto tx = psbt.ExtractTransaction();
    EXPECT_STREQ("02000000000102267ffd76eae6b6c13ffb86ceabaa24d42485520233c2c3805e0ad764709578c00100000017160014962c4e08f336d3afbc3415c9d359ae1040470520ffffffff7d25462a42ff2cb1ba5401e5893259db3ca4bcf5cf6ab9715d35163d31c9ecc0000000006a473044022066ec956e96783cd6122c9c07732a3ae9931b69abdaff1f362c28ed5fa96728d302206a16d4ae46fed48d3d0492737613bd9b51fe3e2453b4446e447c65bf2c967e10012102d9f6888f285a15a6a1880a2202c2b230a42d77cf62da6a48aca4198262cda3c7ffffffff0200e1f50500000000160014b322bddce633b851ac7370ab454f0b367a0654e500e1f50500000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a555024730440220299862a67a9b454d6cda5fee34a52d9089bfa024444d88031c34a98e298bdbca02204b47fcf507b80954108e0375673fdbdd084c5fbb994f6c951720f2eb2f6bb866012102565248460b3c186decf13db06f0724fd50b47cc3e489c30076c8363d5038cefe0000000000", tx.GetHex().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }
}

TEST(Psbt, UsecaseMultisig) {
  Psbt psbt;
  
  HDWallet wallet1 = HDWallet(ByteData(g_psbt_seed1));
  HDWallet wallet2 = HDWallet(ByteData(g_psbt_seed2));
  std::string path1 = "44h/0h/0h/0/12";
  auto key1 = wallet1.GeneratePubkeyData(NetType::kTestnet, path1);
  auto key2 = wallet2.GeneratePubkeyData(NetType::kTestnet, path1);
  auto out_multisig = ScriptUtil::CreateMultisigRedeemScript(
    2, std::vector<Pubkey>{key1.GetPubkey(), key2.GetPubkey()});
  auto addr = Address(NetType::kTestnet, WitnessVersion::kVersion0, out_multisig);

  std::string in_path = "44h/0h/0h/0/11";
  auto key_in1 = wallet1.GeneratePrivkeyData(NetType::kTestnet, in_path);
  auto key_in2 = wallet2.GeneratePrivkeyData(NetType::kTestnet, in_path);
  auto multisig = ScriptUtil::CreateMultisigRedeemScript(
    2, std::vector<Pubkey>{key_in1.GetPubkey(), key_in2.GetPubkey()});
  EXPECT_STREQ("522103a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf32103f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b4752ae", multisig.GetHex().c_str());

  Transaction utxo_tx(g_psbt_utxo_sh_wsh_multi);
  TxIn txin(Txid("544d545a1e53becbf9dd9b0e424e1189b8f6e46d6bc36b191816d341c2d32f69"), 0, 0xffffffff);

  try {
    psbt.AddTxIn(txin);
    psbt.SetTxInUtxo(0, utxo_tx.GetTxOut(0), multisig, KeyData());
    psbt.SetTxInSighashType(0, SigHashType());
    EXPECT_STREQ("522103a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf32103f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b4752ae",
        psbt.GetTxInRedeemScriptDirect(0, true, true).GetHex().c_str());
    EXPECT_STREQ("00209c4dacb25ebb8ada8bbb1addb869dea4d8170cc951f1d9694b2154e1583276c9",
        psbt.GetTxInRedeemScriptDirect(0, true, false).GetHex().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  Amount amount(499993360);
  try {
    psbt.AddTxOut(addr.GetLockingScript(), amount);
    psbt.SetTxOutData(0, out_multisig, std::vector<KeyData>{key1, key2});
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  Psbt psbt1;
  try {
    psbt1.AddTxIn(txin);
    psbt1.SetTxInUtxo(0, utxo_tx.GetTxOut(0), multisig, key_in1);
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  Psbt psbt2;
  try {
    psbt2.AddTxIn(txin);
    psbt2.SetTxInUtxo(0, utxo_tx.GetTxOut(0), multisig, key_in2);
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  try {
    psbt.Join(psbt1);
    psbt.Join(psbt2);
    EXPECT_STREQ("70736274ff01005e0200000001692fd3c241d31618196bc36b6de4f6b889114e420e9bddf9cbbe531e5a544d540000000000ffffffff01104bcd1d000000002200203cad0619de67e6247a76a102813635c053457c6ba4fde4ac1ffd8148d70e4bcc00000000000101200858cd1d0000000017a914945fb50391a70637c1ffc5ab7fb65308c2f23175870103040100000001042200209c4dacb25ebb8ada8bbb1addb869dea4d8170cc951f1d9694b2154e1583276c9010547522103a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf32103f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b4752ae220603a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf3182a7047602c0000800000008000000080000000000b000000220603f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b47189d6b6d862c0000800000008000000080000000000b00000000010147522102906d399f6dbbecc898d4b8de3f497474c86a41f2cf36d71f95e5ca06b074867b2102622c7974ec32de75afa3733b09a6c33d0dea514b29faaccfb0991774f462224252ae220202622c7974ec32de75afa3733b09a6c33d0dea514b29faaccfb0991774f4622242189d6b6d862c0000800000008000000080000000000c000000220202906d399f6dbbecc898d4b8de3f497474c86a41f2cf36d71f95e5ca06b074867b182a7047602c0000800000008000000080000000000c00000000", psbt.GetData().GetHex().c_str());
    EXPECT_STREQ("cHNidP8BAF4CAAAAAWkv08JB0xYYGWvDa23k9riJEU5CDpvd+cu+Ux5aVE1UAAAAAAD/////ARBLzR0AAAAAIgAgPK0GGd5n5iR6dqECgTY1wFNFfGuk/eSsH/2BSNcOS8wAAAAAAAEBIAhYzR0AAAAAF6kUlF+1A5GnBjfB/8Wrf7ZTCMLyMXWHAQMEAQAAAAEEIgAgnE2ssl67itqLuxrduGnepNgXDMlR8dlpSyFU4VgydskBBUdSIQOlEvX1nA55AfxHjtY1Pu929E+cssGFv7z38Lm7dnFr8yED9NRzYU6VSsT1UY5vfLMwe0+EdD4TftTuy19PtkPCO0dSriIGA6US9fWcDnkB/EeO1jU+73b0T5yywYW/vPfwubt2cWvzGCpwR2AsAACAAAAAgAAAAIAAAAAACwAAACIGA/TUc2FOlUrE9VGOb3yzMHtPhHQ+E37U7stfT7ZDwjtHGJ1rbYYsAACAAAAAgAAAAIAAAAAACwAAAAABAUdSIQKQbTmfbbvsyJjUuN4/SXR0yGpB8s821x+V5coGsHSGeyECYix5dOwy3nWvo3M7CabDPQ3qUUsp+qzPsJkXdPRiIkJSriICAmIseXTsMt51r6NzOwmmwz0N6lFLKfqsz7CZF3T0YiJCGJ1rbYYsAACAAAAAgAAAAIAAAAAADAAAACICApBtOZ9tu+zImNS43j9JdHTIakHyzzbXH5XlygawdIZ7GCpwR2AsAACAAAAAgAAAAIAAAAAADAAAAAA=", psbt.GetBase64().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  try {
    psbt1 = psbt;
    psbt1.Sign(key_in1.GetPrivkey());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  try {
    psbt2 = psbt;
    // psbt2.Sign(key_in2.GetPrivkey());
    // Same processing.
    auto sighash_type = psbt.GetTxInSighashType(0);
    auto utxo_txout = psbt.GetTxInUtxo(0);
    auto tx = psbt.GetTransaction();
    auto sighash = tx.GetSignatureHash(0, psbt.GetTxInRedeemScript(0).GetData(),
        sighash_type, utxo_txout.GetValue(), WitnessVersion::kVersion0);
    auto sig = key_in2.GetPrivkey().CalculateEcSignature(sighash);
    sig = CryptoUtil::ConvertSignatureToDer(sig, sighash_type);
    psbt2.SetTxInSignature(0, key_in2, sig);
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  try {
    psbt.Combine(psbt1);
    psbt.Combine(psbt2);
    EXPECT_STREQ("70736274ff01005e0200000001692fd3c241d31618196bc36b6de4f6b889114e420e9bddf9cbbe531e5a544d540000000000ffffffff01104bcd1d000000002200203cad0619de67e6247a76a102813635c053457c6ba4fde4ac1ffd8148d70e4bcc00000000000101200858cd1d0000000017a914945fb50391a70637c1ffc5ab7fb65308c2f2317587220203a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf3473044022020635937e05170d83dc3213adb3a6eae66713008e4abc38b4912c5680dec4f0c022033c08a71a30757b08463d530ec058ed7401968fb30bef63d4549721a0e485b8401220203f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b4747304402205d274a7887de3efade84a4a349c4c36f589b55623b1027b7da6dac89c178563402206a03afebbbb6089f1e37fac8f999a4015161c8920144fd53112da05804cd43cc010103040100000001042200209c4dacb25ebb8ada8bbb1addb869dea4d8170cc951f1d9694b2154e1583276c9010547522103a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf32103f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b4752ae220603a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf3182a7047602c0000800000008000000080000000000b000000220603f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b47189d6b6d862c0000800000008000000080000000000b00000000010147522102906d399f6dbbecc898d4b8de3f497474c86a41f2cf36d71f95e5ca06b074867b2102622c7974ec32de75afa3733b09a6c33d0dea514b29faaccfb0991774f462224252ae220202622c7974ec32de75afa3733b09a6c33d0dea514b29faaccfb0991774f4622242189d6b6d862c0000800000008000000080000000000c000000220202906d399f6dbbecc898d4b8de3f497474c86a41f2cf36d71f95e5ca06b074867b182a7047602c0000800000008000000080000000000c00000000", psbt.GetData().GetHex().c_str());
    EXPECT_STREQ("cHNidP8BAF4CAAAAAWkv08JB0xYYGWvDa23k9riJEU5CDpvd+cu+Ux5aVE1UAAAAAAD/////ARBLzR0AAAAAIgAgPK0GGd5n5iR6dqECgTY1wFNFfGuk/eSsH/2BSNcOS8wAAAAAAAEBIAhYzR0AAAAAF6kUlF+1A5GnBjfB/8Wrf7ZTCMLyMXWHIgIDpRL19ZwOeQH8R47WNT7vdvRPnLLBhb+89/C5u3Zxa/NHMEQCICBjWTfgUXDYPcMhOts6bq5mcTAI5KvDi0kSxWgN7E8MAiAzwIpxowdXsIRj1TDsBY7XQBlo+zC+9j1FSXIaDkhbhAEiAgP01HNhTpVKxPVRjm98szB7T4R0PhN+1O7LX0+2Q8I7R0cwRAIgXSdKeIfePvrehKSjScTDb1ibVWI7ECe32m2sicF4VjQCIGoDr+u7tgifHjf6yPmZpAFRYciSAUT9UxEtoFgEzUPMAQEDBAEAAAABBCIAIJxNrLJeu4rai7sa3bhp3qTYFwzJUfHZaUshVOFYMnbJAQVHUiEDpRL19ZwOeQH8R47WNT7vdvRPnLLBhb+89/C5u3Zxa/MhA/TUc2FOlUrE9VGOb3yzMHtPhHQ+E37U7stfT7ZDwjtHUq4iBgOlEvX1nA55AfxHjtY1Pu929E+cssGFv7z38Lm7dnFr8xgqcEdgLAAAgAAAAIAAAACAAAAAAAsAAAAiBgP01HNhTpVKxPVRjm98szB7T4R0PhN+1O7LX0+2Q8I7Rxida22GLAAAgAAAAIAAAACAAAAAAAsAAAAAAQFHUiECkG05n2277MiY1LjeP0l0dMhqQfLPNtcfleXKBrB0hnshAmIseXTsMt51r6NzOwmmwz0N6lFLKfqsz7CZF3T0YiJCUq4iAgJiLHl07DLeda+jczsJpsM9DepRSyn6rM+wmRd09GIiQhida22GLAAAgAAAAIAAAACAAAAAAAwAAAAiAgKQbTmfbbvsyJjUuN4/SXR0yGpB8s821x+V5coGsHSGexgqcEdgLAAAgAAAAIAAAACAAAAAAAwAAAAA", psbt.GetBase64().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  try {
    auto sig1 = psbt.GetTxInSignature(0, key_in1.GetPubkey());
    auto sig2 = psbt.GetTxInSignature(0, key_in2.GetPubkey());
    auto script = psbt.GetTxInRedeemScript(0);
    EXPECT_FALSE(psbt.IsFinalizedInput(0));
    EXPECT_FALSE(psbt.IsFinalized());
    psbt.SetTxInFinalScript(
        0, std::vector<ByteData>{ByteData(), sig1, sig2, script.GetData()});
    // psbt.Finalize();
    EXPECT_TRUE(psbt.IsFinalizedInput(0));
    psbt.ClearTxInSignData(0);

    auto wit_script = psbt.GetTxInFinalScript(0, true);
    auto wsh_script = psbt.GetTxInFinalScript(0, false);
    EXPECT_EQ(4, wit_script.size());
    if (4 == wit_script.size()) {
      EXPECT_EQ(script.GetHex(), wit_script[3].GetHex());
    }
    EXPECT_EQ(1, wsh_script.size());
    if (1 == wsh_script.size()) {
      auto exp_script = ScriptUtil::CreateP2wshLockingScript(script);
      EXPECT_EQ(exp_script.GetData().Serialize().GetHex(), wsh_script[0].GetHex());
    }
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }

  try {
    EXPECT_TRUE(psbt.IsFinalized());
    auto tx = psbt.ExtractTransaction();
    EXPECT_STREQ("02000000000101692fd3c241d31618196bc36b6de4f6b889114e420e9bddf9cbbe531e5a544d5400000000232200209c4dacb25ebb8ada8bbb1addb869dea4d8170cc951f1d9694b2154e1583276c9ffffffff01104bcd1d000000002200203cad0619de67e6247a76a102813635c053457c6ba4fde4ac1ffd8148d70e4bcc0400473044022020635937e05170d83dc3213adb3a6eae66713008e4abc38b4912c5680dec4f0c022033c08a71a30757b08463d530ec058ed7401968fb30bef63d4549721a0e485b840147304402205d274a7887de3efade84a4a349c4c36f589b55623b1027b7da6dac89c178563402206a03afebbbb6089f1e37fac8f999a4015161c8920144fd53112da05804cd43cc0147522103a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf32103f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b4752ae00000000", tx.GetHex().c_str());
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
    throw except;
  }
}

static constexpr uint8_t kProprietary = 0xfc;

TEST(Psbt, SetGlobalRecordTest) {
  Transaction tx("0200000002267ffd76eae6b6c13ffb86ceabaa24d42485520233c2c3805e0ad764709578c00100000000ffffffff7d25462a42ff2cb1ba5401e5893259db3ca4bcf5cf6ab9715d35163d31c9ecc00000000000ffffffff0200e1f50500000000160014b322bddce633b851ac7370ab454f0b367a0654e500e1f50500000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a55500000000");
  Psbt psbt(tx);

  ByteData global_key1 = Psbt::CreateRecordKey(kProprietary, "cfd", 0, "dummy1");
  ByteData global_value1 = ByteData("01020304");
  ByteData global_key2 = Psbt::CreateRecordKey(kProprietary, "cfd", 0, "dummy2");
  ByteData global_value2 = ByteData("00");
  psbt.SetGlobalRecord(global_key1, global_value1);
  EXPECT_TRUE(psbt.IsFindGlobalRecord(global_key1));
  EXPECT_FALSE(psbt.IsFindGlobalRecord(global_key2));
  psbt.SetGlobalRecord(global_key2, global_value2);
  EXPECT_TRUE(psbt.IsFindGlobalRecord(global_key2));

  EXPECT_STREQ("70736274ff01009a0200000002267ffd76eae6b6c13ffb86ceabaa24d42485520233c2c3805e0ad764709578c00100000000ffffffff7d25462a42ff2cb1ba5401e5893259db3ca4bcf5cf6ab9715d35163d31c9ecc00000000000ffffffff0200e1f50500000000160014b322bddce633b851ac7370ab454f0b367a0654e500e1f50500000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a555000000000dfc03636664000664756d6d793104010203040dfc03636664000664756d6d793201000000000000", psbt.GetData().GetHex().c_str());
  EXPECT_STREQ("cHNidP8BAJoCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8CAOH1BQAAAAAWABSzIr3c5jO4UaxzcKtFTws2egZU5QDh9QUAAAAAFgAUyrjFOm6PwCltHNORWjB9UcSRpVUAAAAADfwDY2ZkAAZkdW1teTEEAQIDBA38A2NmZAAGZHVtbXkyAQAAAAAAAA==", psbt.GetBase64().c_str());

  Psbt psbt2(psbt.GetData());
  auto get_gval1 = psbt2.GetGlobalRecord(global_key1);
  auto get_gval2 = psbt2.GetGlobalRecord(global_key2);
  EXPECT_EQ(get_gval1.GetHex(), global_value1.GetHex());
  EXPECT_EQ(get_gval2.GetHex(), global_value2.GetHex());

  auto key_list = psbt2.GetGlobalRecordKeyList();
  EXPECT_EQ(2, key_list.size());
  if (key_list.size() == 2) {
    EXPECT_EQ(global_key1.GetHex(), key_list[0].GetHex());
    EXPECT_EQ(global_key2.GetHex(), key_list[1].GetHex());
  }
}

TEST(Psbt, SetOutputRecordTest) {
  Psbt psbt("cHNidP8BAEgCAAAAAAIA4fUFAAAAABYAFLMivdzmM7hRrHNwq0VPCzZ6BlTlAOH1BQAAAAAWABTKuMU6bo/AKW0c05FaMH1RxJGlVQAAAAAAAAA=");

  HDWallet wallet1 = HDWallet(ByteData(g_psbt_seed1));
  HDWallet wallet2 = HDWallet(ByteData(g_psbt_seed2));
  std::string path1 = "44h/0h/0h/0/2";
  std::string path2 = "44h/0h/0h/0/2";
  auto key1 = wallet1.GeneratePubkeyData(NetType::kTestnet, path1);
  auto key2 = wallet2.GeneratePubkeyData(NetType::kTestnet, path2);
  auto addr1 = Address(NetType::kTestnet, WitnessVersion::kVersion0, key1.GetPubkey());
  auto addr2 = Address(NetType::kTestnet, WitnessVersion::kVersion0, key2.GetPubkey());

  std::vector<uint8_t> txout_key_bytes(34);
  std::vector<uint8_t> txout_val_bytes;
  txout_key_bytes[0] = 2;
  auto pk_bytes1 = key1.GetPubkey().GetData().GetBytes();
  auto pk_bytes2 = key2.GetPubkey().GetData().GetBytes();
  auto fp1 = key1.GetFingerprint().GetBytes();
  auto fp2 = key2.GetFingerprint().GetBytes();
  auto plist1 = key1.GetChildNumArray();
  auto plist2 = key2.GetChildNumArray();
  memcpy(&txout_key_bytes[1], pk_bytes1.data(), 33);
  txout_val_bytes.resize(4 + (plist1.size() * 4));
  memcpy(txout_val_bytes.data(), fp1.data(), 4);
  memcpy(&txout_val_bytes.data()[4], plist1.data(), 4 * plist1.size());
  ByteData txout_key1 = ByteData(txout_key_bytes);
  ByteData txout_value1 = ByteData(txout_val_bytes);

  memcpy(&txout_key_bytes[1], pk_bytes2.data(), 33);
  txout_val_bytes.resize(4 + (plist2.size() * 4));
  memcpy(txout_val_bytes.data(), fp2.data(), 4);
  memcpy(&txout_val_bytes.data()[4], plist2.data(), 4 * plist2.size());
  ByteData txout_key2 = ByteData(txout_key_bytes);
  ByteData txout_value2 = ByteData(txout_val_bytes);

  psbt.SetTxOutRecord(0, txout_key1, txout_value1);
  // EXPECT_TRUE(psbt.IsFindTxOutRecord(0, txout_key1));
  // EXPECT_FALSE(psbt.IsFindTxOutRecord(1, txout_key2));
  psbt.SetTxOutRecord(1, txout_key2, txout_value2);
  // EXPECT_TRUE(psbt.IsFindTxOutRecord(1, txout_key2));

  EXPECT_STREQ("70736274ff01004802000000000200e1f50500000000160014b322bddce633b851ac7370ab454f0b367a0654e500e1f50500000000160014cab8c53a6e8fc0296d1cd3915a307d51c491a5550000000000220203473bfc8c770c1b220a2e7aae4badf6c0d7eaf29028d5b29d3438012bb289ef81182a7047602c00008000000080000000800000000002000000002202036474aff2633c351865539fb52b62b9d6fb9e4e23576628e1f0a0a7993458e06c189d6b6d862c0000800000008000000080000000000200000000", psbt.GetData().GetHex().c_str());
  EXPECT_STREQ("cHNidP8BAEgCAAAAAAIA4fUFAAAAABYAFLMivdzmM7hRrHNwq0VPCzZ6BlTlAOH1BQAAAAAWABTKuMU6bo/AKW0c05FaMH1RxJGlVQAAAAAAIgIDRzv8jHcMGyIKLnquS632wNfq8pAo1bKdNDgBK7KJ74EYKnBHYCwAAIAAAACAAAAAgAAAAAACAAAAACICA2R0r/JjPDUYZVOftStiudb7nk4jV2Yo4fCgp5k0WOBsGJ1rbYYsAACAAAAAgAAAAIAAAAAAAgAAAAA=", psbt.GetBase64().c_str());

  Psbt psbt2(psbt.GetData());
  auto get_val1 = psbt2.GetTxOutKeyData(0);
  auto get_val2 = psbt2.GetTxOutKeyData(1);
  EXPECT_EQ(get_val1.ToString(), key1.ToString());
  EXPECT_EQ(get_val2.ToString(), key2.ToString());

  ByteData global_key1 = Psbt::CreateRecordKey(kProprietary, "cfd", 0, "dummy1");
  ByteData global_value1 = ByteData("01020304");
  ByteData global_key2 = Psbt::CreateRecordKey(kProprietary, "cfd", 0, "dummy2");
  ByteData global_value2 = ByteData("00");
  psbt.SetTxOutRecord(0, global_key1, global_value1);
  EXPECT_TRUE(psbt.IsFindTxOutRecord(0, global_key1));
  EXPECT_FALSE(psbt.IsFindTxOutRecord(0, global_key2));
  psbt.SetTxOutRecord(0, global_key2, global_value2);
  EXPECT_TRUE(psbt.IsFindTxOutRecord(0, global_key2));

  psbt2 = Psbt(psbt.GetData());
  auto get_gval1 = psbt2.GetTxOutRecord(0, global_key1);
  auto get_gval2 = psbt2.GetTxOutRecord(0, global_key2);
  EXPECT_EQ(get_gval1.GetHex(), global_value1.GetHex());
  EXPECT_EQ(get_gval2.GetHex(), global_value2.GetHex());

  auto key_list = psbt2.GetTxOutRecordKeyList(0);
  EXPECT_EQ(2, key_list.size());
  if (key_list.size() == 2) {
    EXPECT_EQ(global_key1.GetHex(), key_list[0].GetHex());
    EXPECT_EQ(global_key2.GetHex(), key_list[1].GetHex());
  }
  EXPECT_STREQ("cHNidP8BAEgCAAAAAAIA4fUFAAAAABYAFLMivdzmM7hRrHNwq0VPCzZ6BlTlAOH1BQAAAAAWABTKuMU6bo/AKW0c05FaMH1RxJGlVQAAAAAAIgIDRzv8jHcMGyIKLnquS632wNfq8pAo1bKdNDgBK7KJ74EYKnBHYCwAAIAAAACAAAAAgAAAAAACAAAADfwDY2ZkAAZkdW1teTEEAQIDBA38A2NmZAAGZHVtbXkyAQAAIgIDZHSv8mM8NRhlU5+1K2K51vueTiNXZijh8KCnmTRY4GwYnWtthiwAAIAAAACAAAAAgAAAAAACAAAAAA==", psbt2.GetBase64().c_str());
}

TEST(Psbt, SetInputRecordTest) {
  Psbt psbt("cHNidP8BAFwCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8AAAAAAAAAAA==");

  HDWallet wallet1 = HDWallet(ByteData(g_psbt_seed1));
  HDWallet wallet2 = HDWallet(ByteData(g_psbt_seed2));
  std::string path1 = "44h/0h/0h/1/1";
  std::string path2 = "44h/0h/0h/0/1";

  auto key1 = wallet1.GeneratePubkeyData(NetType::kTestnet, path1);
  auto key2 = wallet2.GeneratePubkeyData(NetType::kTestnet, path2);

  std::vector<uint8_t> txin_key_bytes(34);
  std::vector<uint8_t> txin_val_bytes;
  txin_key_bytes[0] = 6;
  auto pk_bytes1 = key1.GetPubkey().GetData().GetBytes();
  auto pk_bytes2 = key2.GetPubkey().GetData().GetBytes();
  auto fp1 = key1.GetFingerprint().GetBytes();
  auto fp2 = key2.GetFingerprint().GetBytes();
  auto plist1 = key1.GetChildNumArray();
  auto plist2 = key2.GetChildNumArray();
  memcpy(&txin_key_bytes[1], pk_bytes1.data(), 33);
  txin_val_bytes.resize(4 + (plist1.size() * 4));
  memcpy(txin_val_bytes.data(), fp1.data(), 4);
  memcpy(&txin_val_bytes.data()[4], plist1.data(), 4 * plist1.size());
  ByteData txout_key1 = ByteData(txin_key_bytes);
  ByteData txout_value1 = ByteData(txin_val_bytes);

  memcpy(&txin_key_bytes[1], pk_bytes2.data(), 33);
  txin_val_bytes.resize(4 + (plist2.size() * 4));
  memcpy(txin_val_bytes.data(), fp2.data(), 4);
  memcpy(&txin_val_bytes.data()[4], plist2.data(), 4 * plist2.size());
  ByteData txout_key2 = ByteData(txin_key_bytes);
  ByteData txout_value2 = ByteData(txin_val_bytes);

  psbt.SetTxInRecord(0, ByteData("00"), Transaction(g_psbt_utxo_witness).GetData());
  try {
    psbt.SetTxInRecord(0, ByteData("01"), ByteData("00e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac97530787"));
    psbt.SetTxInRecord(0, ByteData("03"), ByteData("01000000"));
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
  }
  psbt.SetTxInRecord(0, ByteData("04"), ByteData("0014962c4e08f336d3afbc3415c9d359ae1040470520"));
  psbt.SetTxInRecord(0, txout_key1, txout_value1);

  try {
    psbt.SetTxInRecord(1, ByteData("00"), Transaction(g_psbt_utxo_legacy).GetData());
    psbt.SetTxInRecord(1, txout_key2, txout_value2);
  } catch (const CfdException& except) {
    EXPECT_STREQ("", except.what());
  }

  EXPECT_STREQ("70736274ff01005c0200000002267ffd76eae6b6c13ffb86ceabaa24d42485520233c2c3805e0ad764709578c00100000000ffffffff7d25462a42ff2cb1ba5401e5893259db3ca4bcf5cf6ab9715d35163d31c9ecc00000000000ffffffff0000000000000100f602000000000101f1993fe8e7189542ee4506258e170201be292703cd275acb09ece16672fd848b0000000017160014ac9ef80b27af1c9d95c1db5d761319322bc42fc5ffffffff02080410240100000016001409de2a0431cbb3444fc22cad9d9a0fd09639721000e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac975307870247304402201e07df721c3322419e8f36d07eeae4795975ba0d9d19630ca3cd3dc0d4967172022015428e7be06b6567501539050bd791a380f00bbddbc5097ea97ba7be4017114a0121024aef43b1d5ac7ba5014998d63ceac583959d1fdc66ea2699cd84eeaf82a283060000000001012000e1f5050000000017a914509f5985f4e90a14fb90e39316fdb4f3ac97530787010304010000000104160014962c4e08f336d3afbc3415c9d359ae1040470520220602565248460b3c186decf13db06f0724fd50b47cc3e489c30076c8363d5038cefe182a7047602c00008000000080000000800100000001000000000100bf0200000001c6d2ea36e2e802b52ddac665dacbed2f831b5263459e1ca734f5c945d7515e40000000006a473044022011b96c7d2d0d2e8dcb37138e18acc460752965add9cb8787df5c349df0e2ae6602202e93af31b64f5166e5605819555dabec57be794300fadb37052f31dddea9905c012103e3d244a3967e0b87765fda86c5ff38885f74993953b9584388aef30b26af6aecffffffff017851cd1d000000001976a9148d20443a91969e3bca0e240cd0ffe4dc98c63de288ac00000000220602d9f6888f285a15a6a1880a2202c2b230a42d77cf62da6a48aca4198262cda3c7189d6b6d862c0000800000008000000080000000000100000000", psbt.GetData().GetHex().c_str());
  EXPECT_STREQ("cHNidP8BAFwCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8AAAAAAAABAPYCAAAAAAEB8Zk/6OcYlULuRQYljhcCAb4pJwPNJ1rLCezhZnL9hIsAAAAAFxYAFKye+AsnrxydlcHbXXYTGTIrxC/F/////wIIBBAkAQAAABYAFAneKgQxy7NET8IsrZ2aD9CWOXIQAOH1BQAAAAAXqRRQn1mF9OkKFPuQ45MW/bTzrJdTB4cCRzBEAiAeB99yHDMiQZ6PNtB+6uR5WXW6DZ0ZYwyjzT3A1JZxcgIgFUKOe+BrZWdQFTkFC9eRo4DwC73bxQl+qXunvkAXEUoBIQJK70Ox1ax7pQFJmNY86sWDlZ0f3GbqJpnNhO6vgqKDBgAAAAABASAA4fUFAAAAABepFFCfWYX06QoU+5Djkxb9tPOsl1MHhwEDBAEAAAABBBYAFJYsTgjzNtOvvDQVydNZrhBARwUgIgYCVlJIRgs8GG3s8T2wbwck/VC0fMPkicMAdsg2PVA4zv4YKnBHYCwAAIAAAACAAAAAgAEAAAABAAAAAAEAvwIAAAABxtLqNuLoArUt2sZl2svtL4MbUmNFnhynNPXJRddRXkAAAAAAakcwRAIgEblsfS0NLo3LNxOOGKzEYHUpZa3Zy4eH31w0nfDirmYCIC6TrzG2T1Fm5WBYGVVdq+xXvnlDAPrbNwUvMd3eqZBcASED49JEo5Z+C4d2X9qGxf84iF90mTlTuVhDiK7zCyavauz/////AXhRzR0AAAAAGXapFI0gRDqRlp47yg4kDND/5NyYxj3iiKwAAAAAIgYC2faIjyhaFaahiAoiAsKyMKQtd89i2mpIrKQZgmLNo8cYnWtthiwAAIAAAACAAAAAgAAAAAABAAAAAA==", psbt.GetBase64().c_str());

  Psbt psbt2(psbt.GetData());
  auto get_val1 = psbt2.GetTxInKeyData(0);
  auto get_val2 = psbt2.GetTxInKeyData(1);
  EXPECT_EQ(get_val1.ToString(), key1.ToString());
  EXPECT_EQ(get_val2.ToString(), key2.ToString());

  auto get_tx1 = psbt2.GetTxInUtxoFull(0);
  auto get_tx2 = psbt2.GetTxInUtxoFull(1);
  EXPECT_EQ(get_tx1.GetHex(), g_psbt_utxo_witness);
  EXPECT_EQ(get_tx2.GetHex(), g_psbt_utxo_legacy);

  ByteData global_key1 = Psbt::CreateRecordKey(kProprietary, "cfd", 0, "dummy1");
  ByteData global_value1 = ByteData("01020304");
  ByteData global_key2 = Psbt::CreateRecordKey(kProprietary, "cfd", 0, "dummy2");
  ByteData global_value2 = ByteData("00");
  psbt.SetTxInRecord(0, global_key1, global_value1);
  EXPECT_TRUE(psbt.IsFindTxInRecord(0, global_key1));
  EXPECT_FALSE(psbt.IsFindTxInRecord(0, global_key2));
  psbt.SetTxInRecord(0, global_key2, global_value2);
  EXPECT_TRUE(psbt.IsFindTxInRecord(0, global_key2));

  psbt2 = Psbt(psbt.GetData());
  auto get_gval1 = psbt2.GetTxInRecord(0, global_key1);
  auto get_gval2 = psbt2.GetTxInRecord(0, global_key2);
  EXPECT_EQ(get_gval1.GetHex(), global_value1.GetHex());
  EXPECT_EQ(get_gval2.GetHex(), global_value2.GetHex());

  auto key_list = psbt2.GetTxInRecordKeyList(0);
  EXPECT_EQ(2, key_list.size());
  if (key_list.size() == 2) {
    EXPECT_EQ(global_key1.GetHex(), key_list[0].GetHex());
    EXPECT_EQ(global_key2.GetHex(), key_list[1].GetHex());
  }
  EXPECT_STREQ("cHNidP8BAFwCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8AAAAAAAABAPYCAAAAAAEB8Zk/6OcYlULuRQYljhcCAb4pJwPNJ1rLCezhZnL9hIsAAAAAFxYAFKye+AsnrxydlcHbXXYTGTIrxC/F/////wIIBBAkAQAAABYAFAneKgQxy7NET8IsrZ2aD9CWOXIQAOH1BQAAAAAXqRRQn1mF9OkKFPuQ45MW/bTzrJdTB4cCRzBEAiAeB99yHDMiQZ6PNtB+6uR5WXW6DZ0ZYwyjzT3A1JZxcgIgFUKOe+BrZWdQFTkFC9eRo4DwC73bxQl+qXunvkAXEUoBIQJK70Ox1ax7pQFJmNY86sWDlZ0f3GbqJpnNhO6vgqKDBgAAAAABASAA4fUFAAAAABepFFCfWYX06QoU+5Djkxb9tPOsl1MHhwEDBAEAAAABBBYAFJYsTgjzNtOvvDQVydNZrhBARwUgIgYCVlJIRgs8GG3s8T2wbwck/VC0fMPkicMAdsg2PVA4zv4YKnBHYCwAAIAAAACAAAAAgAEAAAABAAAADfwDY2ZkAAZkdW1teTEEAQIDBA38A2NmZAAGZHVtbXkyAQAAAQC/AgAAAAHG0uo24ugCtS3axmXay+0vgxtSY0WeHKc09clF11FeQAAAAABqRzBEAiARuWx9LQ0ujcs3E44YrMRgdSllrdnLh4ffXDSd8OKuZgIgLpOvMbZPUWblYFgZVV2r7Fe+eUMA+ts3BS8x3d6pkFwBIQPj0kSjln4Lh3Zf2obF/ziIX3SZOVO5WEOIrvMLJq9q7P////8BeFHNHQAAAAAZdqkUjSBEOpGWnjvKDiQM0P/k3JjGPeKIrAAAAAAiBgLZ9oiPKFoVpqGICiICwrIwpC13z2LaakispBmCYs2jxxida22GLAAAgAAAAIAAAACAAAAAAAEAAAAA", psbt2.GetBase64().c_str());
}

TEST(Psbt, GetSignaturePubkeyList) {
  Psbt psbt("cHNidP8BAF4CAAAAAWkv08JB0xYYGWvDa23k9riJEU5CDpvd+cu+Ux5aVE1UAAAAAAD/////ARBLzR0AAAAAIgAgPK0GGd5n5iR6dqECgTY1wFNFfGuk/eSsH/2BSNcOS8wAAAAAAAEBIAhYzR0AAAAAF6kUlF+1A5GnBjfB/8Wrf7ZTCMLyMXWHIgIDpRL19ZwOeQH8R47WNT7vdvRPnLLBhb+89/C5u3Zxa/NHMEQCICBjWTfgUXDYPcMhOts6bq5mcTAI5KvDi0kSxWgN7E8MAiAzwIpxowdXsIRj1TDsBY7XQBlo+zC+9j1FSXIaDkhbhAEiAgP01HNhTpVKxPVRjm98szB7T4R0PhN+1O7LX0+2Q8I7R0cwRAIgXSdKeIfePvrehKSjScTDb1ibVWI7ECe32m2sicF4VjQCIGoDr+u7tgifHjf6yPmZpAFRYciSAUT9UxEtoFgEzUPMAQEDBAEAAAABBCIAIJxNrLJeu4rai7sa3bhp3qTYFwzJUfHZaUshVOFYMnbJAQVHUiEDpRL19ZwOeQH8R47WNT7vdvRPnLLBhb+89/C5u3Zxa/MhA/TUc2FOlUrE9VGOb3yzMHtPhHQ+E37U7stfT7ZDwjtHUq4iBgOlEvX1nA55AfxHjtY1Pu929E+cssGFv7z38Lm7dnFr8xgqcEdgLAAAgAAAAIAAAACAAAAAAAsAAAAiBgP01HNhTpVKxPVRjm98szB7T4R0PhN+1O7LX0+2Q8I7Rxida22GLAAAgAAAAIAAAACAAAAAAAsAAAAAAQAiACA8rQYZ3mfmJHp2oQKBNjXAU0V8a6T95Kwf/YFI1w5LzAEBR1IhApBtOZ9tu+zImNS43j9JdHTIakHyzzbXH5XlygawdIZ7IQJiLHl07DLeda+jczsJpsM9DepRSyn6rM+wmRd09GIiQlKuIgICYix5dOwy3nWvo3M7CabDPQ3qUUsp+qzPsJkXdPRiIkIYnWtthiwAAIAAAACAAAAAgAAAAAAMAAAAIgICkG05n2277MiY1LjeP0l0dMhqQfLPNtcfleXKBrB0hnsYKnBHYCwAAIAAAACAAAAAgAAAAAAMAAAAAA==");

  Psbt psbt2(psbt);
  EXPECT_TRUE(psbt2.IsFindTxInSignature(0, Pubkey("03f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b47")));
  EXPECT_FALSE(psbt2.IsFindTxInSignature(0, Pubkey("03f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b48")));

  auto pk_list = psbt2.GetTxInSignaturePubkeyList(0);
  EXPECT_EQ(2, pk_list.size());
  if (pk_list.size() == 2) {
    EXPECT_STREQ("03a512f5f59c0e7901fc478ed6353eef76f44f9cb2c185bfbcf7f0b9bb76716bf3",
        pk_list[0].GetHex().c_str());
    EXPECT_STREQ("03f4d473614e954ac4f5518e6f7cb3307b4f84743e137ed4eecb5f4fb643c23b47",
        pk_list[1].GetHex().c_str());
  }
}

TEST(Psbt, CreateRecordKey) {
  auto key1 = Psbt::CreateRecordKey(1);
  auto key2 = Psbt::CreateRecordKey(2, ByteData("f1f2"));
  auto key3 = Psbt::CreateRecordKey(3, "abc");
  auto key4 = Psbt::CreateRecordKey(4, ByteData("d1d2"), 5);
  auto key5 = Psbt::CreateRecordKey(5, "def", 9);

  EXPECT_STREQ("01", key1.GetHex().c_str());
  EXPECT_STREQ("0202f1f2", key2.GetHex().c_str());
  EXPECT_STREQ("0303616263", key3.GetHex().c_str());
  EXPECT_STREQ("0402d1d205", key4.GetHex().c_str());
  EXPECT_STREQ("050364656609", key5.GetHex().c_str());
}

TEST(Psbt, GetUtxoDataList) {
  Psbt psbt("cHNidP8BAJoCAAAAAiZ//Xbq5rbBP/uGzquqJNQkhVICM8LDgF4K12RwlXjAAQAAAAD/////fSVGKkL/LLG6VAHliTJZ2zykvPXParlxXTUWPTHJ7MAAAAAAAP////8CAOH1BQAAAAAWABSzIr3c5jO4UaxzcKtFTws2egZU5QDh9QUAAAAAFgAUyrjFOm6PwCltHNORWjB9UcSRpVUAAAAAAAEA9gIAAAAAAQHxmT/o5xiVQu5FBiWOFwIBviknA80nWssJ7OFmcv2EiwAAAAAXFgAUrJ74CyevHJ2VwdtddhMZMivEL8X/////AggEECQBAAAAFgAUCd4qBDHLs0RPwiytnZoP0JY5chAA4fUFAAAAABepFFCfWYX06QoU+5Djkxb9tPOsl1MHhwJHMEQCIB4H33IcMyJBno820H7q5HlZdboNnRljDKPNPcDUlnFyAiAVQo574GtlZ1AVOQUL15GjgPALvdvFCX6pe6e+QBcRSgEhAkrvQ7HVrHulAUmY1jzqxYOVnR/cZuommc2E7q+CooMGAAAAAAEBIADh9QUAAAAAF6kUUJ9ZhfTpChT7kOOTFv2086yXUweHIgICVlJIRgs8GG3s8T2wbwck/VC0fMPkicMAdsg2PVA4zv5HMEQCICmYYqZ6m0VNbNpf7jSlLZCJv6AkRE2IAxw0qY4pi9vKAiBLR/z1B7gJVBCOA3VnP9vdCExfu5lPbJUXIPLrL2u4ZgEBAwQBAAAAAQQWABSWLE4I8zbTr7w0FcnTWa4QQEcFICIGAlZSSEYLPBht7PE9sG8HJP1QtHzD5InDAHbINj1QOM7+GCpwR2AsAACAAAAAgAAAAIABAAAAAQAAAAABAL8CAAAAAcbS6jbi6AK1LdrGZdrL7S+DG1JjRZ4cpzT1yUXXUV5AAAAAAGpHMEQCIBG5bH0tDS6NyzcTjhisxGB1KWWt2cuHh99cNJ3w4q5mAiAuk68xtk9RZuVgWBlVXavsV755QwD62zcFLzHd3qmQXAEhA+PSRKOWfguHdl/ahsX/OIhfdJk5U7lYQ4iu8wsmr2rs/////wF4Uc0dAAAAABl2qRSNIEQ6kZaeO8oOJAzQ/+TcmMY94oisAAAAACICAtn2iI8oWhWmoYgKIgLCsjCkLXfPYtpqSKykGYJizaPHRzBEAiBm7JVulng81hIsnAdzKjrpkxtpq9r/HzYsKO1fqWco0wIgahbUrkb+1I09BJJzdhO9m1H+PiRTtERuRHxlvyyWfhABIgYC2faIjyhaFaahiAoiAsKyMKQtd89i2mpIrKQZgmLNo8cYnWtthiwAAIAAAACAAAAAgAAAAAABAAAAACICA0c7/Ix3DBsiCi56rkut9sDX6vKQKNWynTQ4ASuyie+BGCpwR2AsAACAAAAAgAAAAIAAAAAAAgAAAAAiAgNkdK/yYzw1GGVTn7UrYrnW+55OI1dmKOHwoKeZNFjgbBida22GLAAAgAAAAIAAAACAAAAAAAIAAAAA");
  auto utxos = psbt.GetUtxoDataAll(NetType::kTestnet);
  EXPECT_EQ(2, utxos.size());
  if (utxos.size() == 2) {
    EXPECT_EQ("c078957064d70a5e80c3c23302528524d424aaabce86fb3fc1b6e6ea76fd7f26", utxos[0].txid.GetHex());
    EXPECT_EQ(1, utxos[0].vout);
    EXPECT_EQ("2MzbWwSa1GsmzGLhYbsYRACNhgx4RjqGtTi", utxos[0].address.GetAddress());
    EXPECT_EQ(AddressType::kP2shP2wpkhAddress, utxos[0].address_type);
    EXPECT_EQ(100000000, utxos[0].amount.GetSatoshiValue());
    EXPECT_EQ("sh(wpkh([2a704760/44'/0'/0'/1/1]02565248460b3c186decf13db06f0724fd50b47cc3e489c30076c8363d5038cefe))", utxos[0].descriptor);

    EXPECT_EQ("c0ecc9313d16355d71b96acff5bca43cdb593289e50154bab12cff422a46257d", utxos[1].txid.GetHex());
    EXPECT_EQ(0, utxos[1].vout);
    EXPECT_EQ("mtPAE24cZ3k6NMCcxQJUbSpZnSVaCu96Wj", utxos[1].address.GetAddress());
    EXPECT_EQ(AddressType::kP2pkhAddress, utxos[1].address_type);
    EXPECT_EQ(499995000, utxos[1].amount.GetSatoshiValue());
    EXPECT_EQ("pkh([9d6b6d86/44'/0'/0'/0/1]02d9f6888f285a15a6a1880a2202c2b230a42d77cf62da6a48aca4198262cda3c7)", utxos[1].descriptor);
  }
}