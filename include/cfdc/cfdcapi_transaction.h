/* Copyright 2019 CryptoGarage */
/**
 * @file cfdcapi_transaction.h
 *
 * @brief API definition file of Transaction for used in cfd-capi
 */
#ifndef CFD_INCLUDE_CFDC_CFDCAPI_TRANSACTION_H_
#define CFD_INCLUDE_CFDC_CFDCAPI_TRANSACTION_H_

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif /* __cplusplus */

#include "cfdc/cfdcapi_address.h"
#include "cfdc/cfdcapi_common.h"

/** txin sequence locktime */
enum CfdSequenceLockTime {
  /**
   * @brief disable locktime
   * @deprecated rename to kCfdSequenceLockTimeFinal.
   */
  kCfdSequenceLockTimeDisable = 0xffffffffU,
  /** locktime final */
  kCfdSequenceLockTimeFinal = 0xffffffffU,
  /** enable locktime (maximum time) */
  kCfdSequenceLockTimeEnableMax = 0xfffffffeU,
};

/** tx witness stack type */
enum CfdTxWitnessStackType {
  /** witness stack: normal */
  kCfdTxWitnessStackNormal = 0,
  /** witness stack: pegin (elements) */
  kCfdTxWitnessStackPegin = 1,
};

/** fundrawtransaction option */
enum CfdFundTxOption {
  /** use blind fee (bool) */
  kCfdFundTxIsBlind = 1,
  /** dust fee rate (double) */
  kCfdFundTxDustFeeRate = 2,
  /** longterm fee rate (double) */
  kCfdFundTxLongTermFeeRate = 3,
  /** knapsack min change (int64) */
  kCfdFundTxKnapsackMinChange = 4,
  /** blind option: exponent */
  kCfdFundTxBlindExponent = 5,
  /** blind option: minBits */
  kCfdFundTxBlindMinimumBits = 6,
};

/** code separator default value */
const uint32_t kCfdCodeSeparatorPositionDefault = 0xffffffffU;

/**
 * @brief create initialized transaction.
 * @param[in] handle          cfd handle.
 * @param[in] net_type        network type.
 * @param[in] version         transaction version.
 * @param[in] locktime        lock time.
 * @param[in] tx_hex_string   transaction hex.
 * @param[out] create_handle  create transaction handle.
 *   Call 'CfdFreeCreateTransactionHandle' after you are finished using it.
 * @return CfdErrorCode
 */
CFDC_API int CfdInitializeTransaction(
    void* handle, int net_type, uint32_t version, uint32_t locktime,
    const char* tx_hex_string, void** create_handle);

/**
 * @brief add transaction input.
 * @param[in] handle          cfd handle.
 * @param[in] create_handle   create transaction handle.
 * @param[in] txid            utxo txid.
 * @param[in] vout            utxo vout.
 * @param[in] sequence        sequence number.
 * @return CfdErrorCode
 * @see CfdInitializeTransaction
 */
CFDC_API int CfdAddTransactionInput(
    void* handle, void* create_handle, const char* txid, uint32_t vout,
    uint32_t sequence);

/**
 * @brief add transaction output.
 * @param[in] handle            cfd handle.
 * @param[in] create_handle     create transaction handle.
 * @param[in] value_satoshi     satoshi value. (Specify 0 if disabled)
 * @param[in] address           destination address.(Specify null if disabled)
 * @param[in] direct_locking_script  locking script for direct.
 *                                  (Specify null if disabled.)
 * @param[in] asset_string      value asset.(Specify null if disabled)
 * @return CfdErrorCode
 * @see CfdInitializeTransaction
 */
CFDC_API int CfdAddTransactionOutput(
    void* handle, void* create_handle, int64_t value_satoshi,
    const char* address, const char* direct_locking_script,
    const char* asset_string);

/**
 * @brief split transaction output.
 * @param[in] handle                cfd handle.
 * @param[in] create_handle         create transaction handle.
 * @param[in] split_output_handle   split output handle.
 * @param[in] txout_index           txout index
 * @return CfdErrorCode
 * @see CfdInitializeTransaction
 * @see CfdCreateSplitTxOutHandle
 */
CFDC_API int CfdSplitTxOut(
    void* handle, void* create_handle, void* split_output_handle,
    uint32_t txout_index);

/**
 * @brief update witness stack on transaction input.
 * @param[in] handle            cfd handle.
 * @param[in] create_handle     create transaction handle.
 * @param[in] stack_type        witness stack type.(see: CfdTxWitnessStackType)
 * @param[in] txid              txin txid.
 * @param[in] vout              txin vout.
 * @param[in] stack_index       witness stack index.
 * @param[in] stack_item        update witness stack item.
 * @return CfdErrorCode
 */
CFDC_API int CfdUpdateWitnessStack(
    void* handle, void* create_handle, int stack_type, const char* txid,
    uint32_t vout, uint32_t stack_index, const char* stack_item);

/**
 * @brief clear witness stack on transaction input.
 * @param[in] handle            cfd handle.
 * @param[in] create_handle     create transaction handle.
 * @param[in] txid              txin txid.
 * @param[in] vout              txin vout.
 * @return CfdErrorCode
 */
CFDC_API int CfdClearWitnessStack(
    void* handle, void* create_handle, const char* txid, uint32_t vout);

/**
 * @brief update scriptsig on transaction input.
 * @param[in] handle            cfd handle.
 * @param[in] create_handle     create transaction handle.
 * @param[in] txid              txin txid.
 * @param[in] vout              txin vout.
 * @param[in] script_sig        unlocking script(script signature).
 * @return CfdErrorCode
 */
CFDC_API int CfdUpdateTxInScriptSig(
    void* handle, void* create_handle, const char* txid, uint32_t vout,
    const char* script_sig);

/**
 * @brief update sequence on transaction input.
 * @param[in] handle            cfd handle.
 * @param[in] create_handle     create transaction handle.
 * @param[in] txid              txin txid.
 * @param[in] vout              txin vout.
 * @param[in] sequence          sequence number.
 * @return CfdErrorCode
 */
CFDC_API int CfdUpdateTxInSequence(
    void* handle, void* create_handle, const char* txid, uint32_t vout,
    uint32_t sequence);

/**
 * @brief set utxo data on transaction input.
 * @details for calculate schnorr sighature.
 * @param[in] handle            cfd handle.
 * @param[in] create_handle     create transaction handle.
 * @param[in] txid              utxo txid.
 * @param[in] vout              utxo vout.
 * @param[in] amount            utxo amount
 * @param[in] commitment        utxo amount commitment
 * @param[in] descriptor        utxo descriptor
 * @param[in] address           utxo address
 * @param[in] asset             utxo asset
 * @param[in] scriptsig_template    utxo scriptsig template
 * @param[in] can_insert        insert mode
 * @return CfdErrorCode
 */
CFDC_API int CfdSetTransactionUtxoData(
    void* handle, void* create_handle, const char* txid, uint32_t vout,
    int64_t amount, const char* commitment, const char* descriptor,
    const char* address, const char* asset, const char* scriptsig_template,
    bool can_insert);

/**
 * @brief Create sighash on transaction input.
 * @details Call CfdSetTransactionUtxoData before calling this function.
 * @param[in] handle            cfd handle.
 * @param[in] create_handle     create transaction handle.
 * @param[in] txid              txin txid.
 * @param[in] vout              txin vout.
 * @param[in] sighash_type      sighash type.
 * @param[in] sighash_anyone_can_pay    anyone can pay flag.
 * @param[in] pubkey            pubkey
 * @param[in] redeem_script     redeem script
 * @param[in] tapleaf_hash      tapleaf hash
 * @param[in] code_separator_position   code separator position
 * @param[in] annex             annex bytes.
 * @param[out] sighash          sighash
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @return CfdErrorCode
 */
CFDC_API int CfdCreateSighashByHandle(
    void* handle, void* create_handle, const char* txid, uint32_t vout,
    int sighash_type, bool sighash_anyone_can_pay, const char* pubkey,
    const char* redeem_script, const char* tapleaf_hash,
    uint32_t code_separator_position, const char* annex, char** sighash);

/**
 * @brief Sign with private key on transaction input.
 * @details Call CfdSetTransactionUtxoData before calling this function.
 * @param[in] handle            cfd handle.
 * @param[in] create_handle     create transaction handle.
 * @param[in] txid              txin txid.
 * @param[in] vout              txin vout.
 * @param[in] privkey           private key.
 * @param[in] sighash_type      sighash type.
 * @param[in] sighash_anyone_can_pay    anyone can pay flag.
 * @param[in] has_grind_r       Grind-R flag on sign.
 * @param[in] aux_rand          the auxiliary random data.\
 *      used to create the nonce.
 * @param[in] annex             annex bytes.
 * @return CfdErrorCode
 */
CFDC_API int CfdAddSignWithPrivkeyByHandle(
    void* handle, void* create_handle, const char* txid, uint32_t vout,
    const char* privkey, int sighash_type, bool sighash_anyone_can_pay,
    bool has_grind_r, const char* aux_rand, const char* annex);

/**
 * @brief Verify transactin sign. (It does not check the Script itself.)
 * @details Call CfdSetTransactionUtxoData before calling this function.
 * @param[in] handle            cfd handle.
 * @param[in] create_handle     create transaction handle.
 * @param[in] txid              txin txid.
 * @param[in] vout              txin vout.
 * @return CfdErrorCode
 */
CFDC_API int CfdVerifyTxSignByHandle(
    void* handle, void* create_handle, const char* txid, uint32_t vout);

/**
 * @brief Add tx sign on transaction input.
 * @param[in] handle            cfd handle.
 * @param[in] create_handle     create transaction handle.
 * @param[in] txid              txin txid.
 * @param[in] vout              txin vout.
 * @param[in] hash_type         hash type.
 * @param[in] sign_data_hex     sign data.
 * @param[in] use_der_encode    use der encode.
 * @param[in] sighash_type      sighash type.
 * @param[in] sighash_anyone_can_pay    anyone can pay flag.
 * @param[in] clear_stack       clear witness stack.
 * @return CfdErrorCode
 */
CFDC_API int CfdAddTxSignByHandle(
    void* handle, void* create_handle, const char* txid, uint32_t vout,
    int hash_type, const char* sign_data_hex, bool use_der_encode,
    int sighash_type, bool sighash_anyone_can_pay, bool clear_stack);

/**
 * @brief Add tx taproot sign on transaction input.
 * @param[in] handle            cfd handle.
 * @param[in] create_handle     create transaction handle.
 * @param[in] txid              txin txid.
 * @param[in] vout              txin vout.
 * @param[in] signature         signature.
 *      It is assumed that sighashType is assigned \
 *      by CfdAddSighashTypeInSchnorrSignature().
 * @param[in] tapscript         tapscript hex.
 * @param[in] control_block     taproot control block.
 *      see CfdGetTaprootScriptTreeHash().
 * @param[in] annex             annex bytes.
 * @return CfdErrorCode
 */
CFDC_API int CfdAddTaprootSignByHandle(
    void* handle, void* create_handle, const char* txid, uint32_t vout,
    const char* signature, const char* tapscript, const char* control_block,
    const char* annex);

/**
 * @brief Add tx pubkey hash sign on transaction input.
 * @param[in] handle            cfd handle.
 * @param[in] create_handle     create transaction handle.
 * @param[in] txid              txin txid.
 * @param[in] vout              txin vout.
 * @param[in] hash_type         hash type.
 * @param[in] pubkey            pubkey.
 * @param[in] signature         signature.
 * @param[in] use_der_encode    use der encode.
 * @param[in] sighash_type      sighash type.
 * @param[in] sighash_anyone_can_pay    anyone can pay flag.
 * @return CfdErrorCode
 */
CFDC_API int CfdAddPubkeyHashSignByHandle(
    void* handle, void* create_handle, const char* txid, uint32_t vout,
    int hash_type, const char* pubkey, const char* signature,
    bool use_der_encode, int sighash_type, bool sighash_anyone_can_pay);

/**
 * @brief Add tx script hash final sign on transaction input.
 * @param[in] handle            cfd handle.
 * @param[in] create_handle     create transaction handle.
 * @param[in] txid              txin txid.
 * @param[in] vout              txin vout.
 * @param[in] hash_type         hash type.
 * @param[in] redeem_script     redeem script.
 * @return CfdErrorCode
 */
CFDC_API int CfdAddScriptHashLastSignByHandle(
    void* handle, void* create_handle, const char* txid, uint32_t vout,
    int hash_type, const char* redeem_script);

/**
 * @brief finalize and execute createrawtransaction.
 * @param[in] handle            cfd handle.
 * @param[in] create_handle     create transaction handle.
 * @param[out] tx_hex_string    tx hex.
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @return CfdErrorCode
 * @see CfdInitializeTransaction
 */
CFDC_API int CfdFinalizeTransaction(
    void* handle, void* create_handle, char** tx_hex_string);

/**
 * @brief free create transaction handle.
 * @param[in] handle          handle pointer.
 * @param[in] create_handle   create transaction handle.
 * @return CfdErrorCode
 * @see CfdInitializeTransaction
 */
CFDC_API int CfdFreeTransactionHandle(void* handle, void* create_handle);

/**
 * @brief create split output handle.
 * @param[in] handle                cfd handle.
 * @param[in] create_handle         create transaction handle.
 * @param[out] split_output_handle  split output handle.
 *   Call 'CfdFreeSplitTxOutHandle' after you are finished using it.
 * @return CfdErrorCode
 * @see CfdSplitTxOut
 */
CFDC_API int CfdCreateSplitTxOutHandle(
    void* handle, void* create_handle, void** split_output_handle);

/**
 * @brief Add split txout data.
 * @param[in] handle                    cfd handle.
 * @param[in] split_output_handle       split output handle.
 * @param[in] amount                    amount.
 * @param[in] address                   address. (or confidential address)
 * @param[in] direct_locking_script     direct locking script.
 * @param[in] direct_nonce              direct nonce.
 * @return CfdErrorCode
 * @see CfdSplitTxOut
 */
CFDC_API int CfdAddSplitTxOutData(
    void* handle, void* split_output_handle, int64_t amount,
    const char* address, const char* direct_locking_script,
    const char* direct_nonce);

/**
 * @brief free split output handle.
 * @param[in] handle                cfd handle.
 * @param[in] split_output_handle   split output handle.
 * @return CfdErrorCode
 * @see CfdSplitTxOut
 */
CFDC_API int CfdFreeSplitTxOutHandle(void* handle, void* split_output_handle);

/**
 * @brief update txout's amount.
 * @param[in] handle          cfd handle.
 * @param[in] net_type        network type.
 * @param[in] tx_hex_string   transaction hex.
 * @param[in] index           txout index.
 * @param[in] amount          txout amount.
 * @param[out] tx_string      modify transaction hex.
 * @return CfdErrorCode
 */
CFDC_API int CfdUpdateTxOutAmount(
    void* handle, int net_type, const char* tx_hex_string, uint32_t index,
    int64_t amount, char** tx_string);

/**
 * @brief add sign to transaction.
 * @param[in] handle          cfd handle.
 * @param[in] net_type        network type.
 * @param[in] tx_hex_string   transaction hex.
 * @param[in] txid            txin txid.
 * @param[in] vout            txin vout.
 * @param[in] hash_type       hash type.
 * @param[in] sign_data_hex   add sign data hex.
 * @param[in] use_der_encode  der encode flag.
 * @param[in] sighash_type            sighash type.
 * @param[in] sighash_anyone_can_pay  sighash anyone can pay flag.
 * @param[in] clear_stack     cleanup scriptSig stack data.
 * @param[out] tx_string      signed transaction hex.
 * @return CfdErrorCode
 */
CFDC_API int CfdAddTxSign(
    void* handle, int net_type, const char* tx_hex_string, const char* txid,
    uint32_t vout, int hash_type, const char* sign_data_hex,
    bool use_der_encode, int sighash_type, bool sighash_anyone_can_pay,
    bool clear_stack, char** tx_string);

/**
 * @brief add sign signature and pubkey to transaction.
 * @param[in] handle          cfd handle.
 * @param[in] net_type        network type.
 * @param[in] tx_hex_string   transaction hex.
 * @param[in] txid            txin txid.
 * @param[in] vout            txin vout.
 * @param[in] hash_type       hash type.
 * @param[in] pubkey          pubkey hex.
 * @param[in] signature       add sign data signature.
 * @param[in] use_der_encode  der encode flag.
 * @param[in] sighash_type            sighash type.
 * @param[in] sighash_anyone_can_pay  sighash anyone can pay flag.
 * @param[out] tx_string      signed transaction hex.
 * @return CfdErrorCode
 */
CFDC_API int CfdAddPubkeyHashSign(
    void* handle, int net_type, const char* tx_hex_string, const char* txid,
    uint32_t vout, int hash_type, const char* pubkey, const char* signature,
    bool use_der_encode, int sighash_type, bool sighash_anyone_can_pay,
    char** tx_string);

/**
 * @brief add redeem script to transaction.
 * @details using after CfdAddTxSign.
 * @param[in] handle          cfd handle.
 * @param[in] net_type        network type.
 * @param[in] tx_hex_string   transaction hex.
 * @param[in] txid            txin txid.
 * @param[in] vout            txin vout.
 * @param[in] hash_type       hash type.
 * @param[in] redeem_script   redeem script.
 * @param[in] clear_stack     cleanup scriptSig stack data.
 * @param[out] tx_string      signed transaction hex.
 * @return CfdErrorCode
 */
CFDC_API int CfdAddScriptHashSign(
    void* handle, int net_type, const char* tx_hex_string, const char* txid,
    uint32_t vout, int hash_type, const char* redeem_script, bool clear_stack,
    char** tx_string);

/**
 * @brief add sign with privkey.
 * @param[in] handle          cfd handle.
 * @param[in] net_type        network type.
 * @param[in] tx_hex_string   transaction hex.
 * @param[in] txid            txin txid.
 * @param[in] vout            txin vout.
 * @param[in] hash_type       hash type.(p2pkh, p2wpkh, p2sh-p2wpkh)
 * @param[in] pubkey          pubkey hex.
 * @param[in] privkey         privkey (WIF or hex).
 * @param[in] value_satoshi   satoshi value. (Specify 0 if disabled)
 * @param[in] sighash_type            sighash type.
 * @param[in] sighash_anyone_can_pay  sighash anyone can pay flag.
 * @param[in] has_grind_r     ec-signature Grind-R flag.
 * @param[out] tx_string      signed transaction hex.
 * @return CfdErrorCode
 */
CFDC_API int CfdAddSignWithPrivkeySimple(
    void* handle, int net_type, const char* tx_hex_string, const char* txid,
    uint32_t vout, int hash_type, const char* pubkey, const char* privkey,
    int64_t value_satoshi, int sighash_type, bool sighash_anyone_can_pay,
    bool has_grind_r, char** tx_string);

/**
 * @brief initialized for multisig sign.
 * @param[in] handle          cfd handle.
 * @param[out] multisign_handle  multisig sign handle.
 *   Call 'CfdFreeMultisigSignHandle' after you are finished using it.
 * @return CfdErrorCode
 */
CFDC_API int CfdInitializeMultisigSign(void* handle, void** multisign_handle);

/**
 * @brief append for multisig signature data.
 * @param[in] handle            cfd handle.
 * @param[in] multisign_handle  multisig sign handle.
 * @param[in] signature         signature hex.
 * @param[in] related_pubkey    pubkey hex.
 * @return CfdErrorCode
 */
CFDC_API int CfdAddMultisigSignData(
    void* handle, void* multisign_handle, const char* signature,
    const char* related_pubkey);

/**
 * @brief append for multisig signature data.
 * @param[in] handle                  cfd handle.
 * @param[in] multisign_handle        multisig sign handle.
 * @param[in] signature               signature hex.
 * @param[in] sighash_type            sighash type.
 * @param[in] sighash_anyone_can_pay  sighash anyone can pey flag.
 * @param[in] related_pubkey          pubkey hex.
 * @return CfdErrorCode
 */
CFDC_API int CfdAddMultisigSignDataToDer(
    void* handle, void* multisign_handle, const char* signature,
    int sighash_type, bool sighash_anyone_can_pay, const char* related_pubkey);

/**
 * @brief append multisig sign to transaction.
 * @param[in] handle            cfd handle.
 * @param[in] multisign_handle  multisig sign handle.
 * @param[in] net_type          network type.
 * @param[in] tx_hex_string     tx hex.
 * @param[in] txid              txin txid.
 * @param[in] vout              txin vout.
 * @param[in] hash_type         hash type.
 * @param[in] redeem_script     multisig redeem script.
 * @param[out] tx_string        signed tx hex.
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @return CfdErrorCode
 */
CFDC_API int CfdFinalizeMultisigSign(
    void* handle, void* multisign_handle, int net_type,
    const char* tx_hex_string, const char* txid, uint32_t vout, int hash_type,
    const char* redeem_script, char** tx_string);

/**
 * @brief free multisig sign handle.
 * @param[in] handle            handle pointer.
 * @param[in] multisign_handle  multisig sign handle.
 * @return CfdErrorCode
 */
CFDC_API int CfdFreeMultisigSignHandle(void* handle, void* multisign_handle);

/**
 * @brief verify signature data.
 * @param[in] handle                  cfd handle.
 * @param[in] net_type                network type.
 * @param[in] tx_hex                  transaction hex.
 * @param[in] signature               signature hex.
 * @param[in] hash_type               hash type.
 * @param[in] pubkey                  pubkey hex.
 * @param[in] script                  script hex. (Specify NULL if disabled)
 * @param[in] txid                    txid.
 * @param[in] vout                    vout.
 * @param[in] sighash_type            sighash type. (ref: CfdSighashType)
 * @param[in] sighash_anyone_can_pay  sighash anyone can pay flag.
 * @param[in] value_satoshi           value satoshi. (Specify 0 if disabled)
 * @param[in] value_bytedata          value commitment.
 *     (Specify null if disabled)
 * @return CfdErrorCode
 *     (if failed to verify signature, it returns kCfdSignVerificationError)
 */
CFDC_API int CfdVerifySignature(
    void* handle, int net_type, const char* tx_hex, const char* signature,
    int hash_type, const char* pubkey, const char* script, const char* txid,
    uint32_t vout, int sighash_type, bool sighash_anyone_can_pay,
    int64_t value_satoshi, const char* value_bytedata);

/**
 * @brief verify transaction's scriptsig or witness stack.
 * @details support type is p2pkh, p2wpkh, p2sh-p2wpkh,
 *     multisig(for p2sh, p2wsh, p2sh-p2wsh)
 * @param[in] handle                  cfd handle.
 * @param[in] net_type                network type.
 * @param[in] tx_hex                  transaction hex.
 * @param[in] txid                    txid.
 * @param[in] vout                    vout.
 * @param[in] address                 utxo address.
 * @param[in] address_type            address type. (ref: CfdAddressType)
 * @param[in] direct_locking_script   utxo locking script.
 *     (set when address is empty.)
 * @param[in] value_satoshi           value satoshi. (Specify 0 if disabled)
 * @param[in] value_bytedata          value commitment.
 *     (Specify null if disabled)
 * @return CfdErrorCode
 *     (if failed to verify signature, it returns kCfdSignVerificationError)
 */
CFDC_API int CfdVerifyTxSign(
    void* handle, int net_type, const char* tx_hex, const char* txid,
    uint32_t vout, const char* address, int address_type,
    const char* direct_locking_script, int64_t value_satoshi,
    const char* value_bytedata);

/**
 * @brief create transaction sighash.
 * @param[in] handle            cfd handle.
 * @param[in] net_type          network type.
 * @param[in] tx_hex_string     tx hex.
 * @param[in] txid              txin txid.
 * @param[in] vout              txin vout.
 * @param[in] hash_type         hash type.
 * @param[in] pubkey            pubkey. (Specify null if disabled)
 * @param[in] redeem_script     redeem script. (Specify null if disabled)
 * @param[in] value_satoshi     satoshi value. (Specify 0 if disabled)
 * @param[in] sighash_type            sighash type.
 * @param[in] sighash_anyone_can_pay  sighash anyone can pay flag.
 * @param[out] sighash          sighash.
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @return CfdErrorCode
 */
CFDC_API int CfdCreateSighash(
    void* handle, int net_type, const char* tx_hex_string, const char* txid,
    uint32_t vout, int hash_type, const char* pubkey,
    const char* redeem_script, int64_t value_satoshi, int sighash_type,
    bool sighash_anyone_can_pay, char** sighash);

/**
 * @brief get transaction information.
 * @param[in] handle            cfd handle.
 * @param[in] net_type          network type.
 * @param[in] tx_hex_string     transaction hex.
 * @param[out] txid             transaction id.
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @param[out] wtxid            witness transaction id.
 *   If no-witness transaction, return is txid.
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @param[out] size             transaction size.
 * @param[out] vsize            virtual transaction size.
 * @param[out] weight           weight.
 * @param[out] version          transaction version.
 * @param[out] locktime         transaction locktime.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxInfo(
    void* handle, int net_type, const char* tx_hex_string, char** txid,
    char** wtxid, uint32_t* size, uint32_t* vsize, uint32_t* weight,
    uint32_t* version, uint32_t* locktime);

/**
 * @brief get transaction input.
 * @param[in] handle            cfd handle.
 * @param[in] net_type          network type.
 * @param[in] tx_hex_string     transaction hex.
 * @param[in] index             txin index.
 * @param[out] txid             transaction id.
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @param[out] vout             transaction vout
 * @param[out] sequence         sequence number.
 * @param[out] script_sig       unlocking script(script signature).
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxIn(
    void* handle, int net_type, const char* tx_hex_string, uint32_t index,
    char** txid, uint32_t* vout, uint32_t* sequence, char** script_sig);

/**
 * @brief get transaction input witness stack.
 * @param[in] handle            cfd handle.
 * @param[in] net_type          network type.
 * @param[in] tx_hex_string     transaction hex.
 * @param[in] txin_index        txin index.
 * @param[in] stack_index       witness stack index.
 * @param[out] stack_data       witness stack data.
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxInWitness(
    void* handle, int net_type, const char* tx_hex_string, uint32_t txin_index,
    uint32_t stack_index, char** stack_data);

/**
 * @brief get transaction output.
 * @param[in] handle            cfd handle.
 * @param[in] net_type          network type.
 * @param[in] tx_hex_string     transaction hex.
 * @param[in] index             txout index.
 * @param[out] value_satoshi     satoshi value.
 * @param[out] locking_script    locking script
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxOut(
    void* handle, int net_type, const char* tx_hex_string, uint32_t index,
    int64_t* value_satoshi, char** locking_script);

/**
 * @brief get transaction input count.
 * @param[in] handle            cfd handle.
 * @param[in] net_type          network type.
 * @param[in] tx_hex_string     transaction hex.
 * @param[out] count            txin count.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxInCount(
    void* handle, int net_type, const char* tx_hex_string, uint32_t* count);

/**
 * @brief get transaction input witness stack count.
 * @param[in] handle            cfd handle.
 * @param[in] net_type          network type.
 * @param[in] tx_hex_string     transaction hex.
 * @param[in] txin_index        txin index.
 * @param[out] count            witness stack count.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxInWitnessCount(
    void* handle, int net_type, const char* tx_hex_string, uint32_t txin_index,
    uint32_t* count);

/**
 * @brief get transaction output count.
 * @param[in] handle            cfd handle.
 * @param[in] net_type          network type.
 * @param[in] tx_hex_string     transaction hex.
 * @param[out] count            txout count.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxOutCount(
    void* handle, int net_type, const char* tx_hex_string, uint32_t* count);

/**
 * @brief get tx-input index.
 * @param[in] handle          cfd handle.
 * @param[in] net_type        network type.
 * @param[in] tx_hex_string   tx hex.
 * @param[in] txid            txin txid.
 * @param[in] vout            txin vout.
 * @param[out] index          txin index.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxInIndex(
    void* handle, int net_type, const char* tx_hex_string, const char* txid,
    uint32_t vout, uint32_t* index);

/**
 * @brief get tx-output index.
 * @param[in] handle                 cfd handle.
 * @param[in] net_type               network type.
 * @param[in] tx_hex_string          tx hex.
 * @param[in] address                txout address.
 * @param[in] direct_locking_script  txout locking script. (not use address)
 * @param[out] index                 txout index.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxOutIndex(
    void* handle, int net_type, const char* tx_hex_string, const char* address,
    const char* direct_locking_script, uint32_t* index);

/**
 * @brief Initialize handle for transaction data.
 * @param[in] handle              cfd handle.
 * @param[in] net_type            network type.
 * @param[in] tx_hex_string       tx hex.
 * @param[out] tx_data_handle     transaction data handle.
 * @return CfdErrorCode
 */
CFDC_API int CfdInitializeTxDataHandle(
    void* handle, int net_type, const char* tx_hex_string,
    void** tx_data_handle);

/**
 * @brief free transaction data handle.
 * @param[in] handle            handle pointer.
 * @param[in] tx_data_handle    transaction data handle.
 * @return CfdErrorCode
 */
CFDC_API int CfdFreeTxDataHandle(void* handle, void* tx_data_handle);

/**
 * @brief free transaction data handle.
 * @param[in] handle            handle pointer.
 * @param[in] tx_data_handle    transaction data handle.
 * @param[out] tx_hex_string    transaction hex.
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetModifiedTxByHandle(
    void* handle, void* tx_data_handle, char** tx_hex_string);

/**
 * @brief get transaction information.
 * @param[in] handle            cfd handle.
 * @param[in] tx_data_handle    transaction data handle.
 * @param[out] txid             transaction id.
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @param[out] wtxid            witness transaction id.
 *   If no-witness transaction, return is txid.
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @param[out] size             transaction size.
 * @param[out] vsize            virtual transaction size.
 * @param[out] weight           weight.
 * @param[out] version          transaction version.
 * @param[out] locktime         transaction locktime.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxInfoByHandle(
    void* handle, void* tx_data_handle, char** txid, char** wtxid,
    uint32_t* size, uint32_t* vsize, uint32_t* weight, uint32_t* version,
    uint32_t* locktime);

/**
 * @brief get transaction input.
 * @param[in] handle            cfd handle.
 * @param[in] tx_data_handle    transaction data handle.
 * @param[in] index             txin index.
 * @param[out] txid             transaction id.
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @param[out] vout             transaction vout
 * @param[out] sequence         sequence number.
 * @param[out] script_sig       unlocking script(script signature).
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxInByHandle(
    void* handle, void* tx_data_handle, uint32_t index, char** txid,
    uint32_t* vout, uint32_t* sequence, char** script_sig);

/**
 * @brief get transaction input witness stack.
 * @param[in] handle            cfd handle.
 * @param[in] tx_data_handle    transaction data handle.
 * @param[in] stack_type        witness stack type.(see: CfdTxWitnessStackType)
 * @param[in] txin_index        txin index.
 * @param[in] stack_index       witness stack index.
 * @param[out] stack_data       witness stack data.
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxInWitnessByHandle(
    void* handle, void* tx_data_handle, int stack_type, uint32_t txin_index,
    uint32_t stack_index, char** stack_data);

/**
 * @brief get transaction output.
 * @param[in] handle            cfd handle.
 * @param[in] tx_data_handle    transaction data handle.
 * @param[in] index             txout index.
 * @param[out] value_satoshi    satoshi value.
 * @param[out] locking_script   locking script
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @param[out] asset            asset (elements only)
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxOutByHandle(
    void* handle, void* tx_data_handle, uint32_t index, int64_t* value_satoshi,
    char** locking_script, char** asset);

/**
 * @brief get transaction input count.
 * @param[in] handle            cfd handle.
 * @param[in] tx_data_handle    transaction data handle.
 * @param[out] count            txin count.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxInCountByHandle(
    void* handle, void* tx_data_handle, uint32_t* count);

/**
 * @brief get transaction input witness stack count.
 * @param[in] handle            cfd handle.
 * @param[in] tx_data_handle    transaction data handle.
 * @param[in] stack_type        witness stack type.(see: CfdTxWitnessStackType)
 * @param[in] txin_index        txin index.
 * @param[out] count            witness stack count.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxInWitnessCountByHandle(
    void* handle, void* tx_data_handle, int stack_type, uint32_t txin_index,
    uint32_t* count);

/**
 * @brief get transaction output count.
 * @param[in] handle            cfd handle.
 * @param[in] tx_data_handle    transaction data handle.
 * @param[out] count            txout count.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxOutCountByHandle(
    void* handle, void* tx_data_handle, uint32_t* count);

/**
 * @brief get tx-input index.
 * @param[in] handle            cfd handle.
 * @param[in] tx_data_handle    transaction data handle.
 * @param[in] txid              txin txid.
 * @param[in] vout              txin vout.
 * @param[out] index            txin index.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxInIndexByHandle(
    void* handle, void* tx_data_handle, const char* txid, uint32_t vout,
    uint32_t* index);

/**
 * @brief get tx-output index.
 * @param[in] handle                 cfd handle.
 * @param[in] tx_data_handle         transaction data handle.
 * @param[in] address                txout address.
 * @param[in] direct_locking_script  txout locking script. (not use address)
 * @param[out] index                 txout index.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxOutIndexByHandle(
    void* handle, void* tx_data_handle, const char* address,
    const char* direct_locking_script, uint32_t* index);

/**
 * @brief get tx-output index with offset.
 * @param[in] handle                 cfd handle.
 * @param[in] tx_data_handle         transaction data handle.
 * @param[in] offset                 search start offset.
 * @param[in] address                txout address.
 * @param[in] direct_locking_script  txout locking script. (not use address)
 * @param[out] index                 txout index.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetTxOutIndexWithOffsetByHandle(
    void* handle, void* tx_data_handle, uint32_t offset, const char* address,
    const char* direct_locking_script, uint32_t* index);

/**
 * @brief Initialize handle for fundrawtransaction.
 * @param[in] handle              cfd handle.
 * @param[in] network_type        network type.
 * @param[in] target_asset_count  target asset count.
 * @param[in] fee_asset           fee asset. (elements only)
 * @param[out] fund_handle        handle for fundrawtransaction.
 * @return CfdErrorCode
 */
CFDC_API int CfdInitializeFundRawTx(
    void* handle, int network_type, uint32_t target_asset_count,
    const char* fee_asset, void** fund_handle);

/**
 * @brief Add transaction input's utxo for fundrawtransaction.
 * @param[in] handle             cfd handle.
 * @param[in] fund_handle        handle for fundrawtransaction.
 * @param[in] txid               input utxo's transaction id.
 * @param[in] vout               input utxo's vout.
 * @param[in] amount             input utxo's amount.
 * @param[in] descriptor         the descriptor for creating locking_script.
 * @param[in] asset              utxo's unblind asset id.
 * @param[in] is_issuance        is utxo issuance input.
 * @param[in] is_blind_issuance  is utxo blind issuance input.
 * @param[in] is_pegin           is utxo pegin input.
 * @param[in] pegin_btc_tx_size  size of pegin transaction.
 * @param[in] claim_script       utxo's claim script.
 * @return CfdErrorCode
 * @deprecated Please use CfdAddTxInputForFundRawTx
 */
CFDC_API int CfdAddTxInForFundRawTx(
    void* handle, void* fund_handle, const char* txid, uint32_t vout,
    int64_t amount, const char* descriptor, const char* asset,
    bool is_issuance, bool is_blind_issuance, bool is_pegin,
    uint32_t pegin_btc_tx_size, const char* claim_script);

/**
 * @brief Add transaction input's utxo for fundrawtransaction.
 * @param[in] handle              cfd handle.
 * @param[in] fund_handle         handle for fundrawtransaction.
 * @param[in] txid                input utxo's transaction id.
 * @param[in] vout                input utxo's vout.
 * @param[in] amount              input utxo's amount.
 * @param[in] descriptor          the descriptor for creating locking_script.
 * @param[in] asset               utxo's unblind asset id.
 * @param[in] is_issuance         is utxo issuance input.
 * @param[in] is_blind_issuance   is utxo blind issuance input.
 * @param[in] is_pegin            is utxo pegin input.
 * @param[in] pegin_btc_tx_size   size of pegin transaction.
 * @param[in] claim_script       utxo's claim script.
 * @param[in] scriptsig_template  utxo's scriptsig template.
 * @return CfdErrorCode
 * @deprecated Please use CfdAddTxInputForFundRawTx
 */
CFDC_API int CfdAddTxInTemplateForFundRawTx(
    void* handle, void* fund_handle, const char* txid, uint32_t vout,
    int64_t amount, const char* descriptor, const char* asset,
    bool is_issuance, bool is_blind_issuance, bool is_pegin,
    uint32_t pegin_btc_tx_size, const char* claim_script,
    const char* scriptsig_template);

/**
 * @brief Add transaction input's utxo for fundrawtransaction.
 * @param[in] handle              cfd handle.
 * @param[in] fund_handle         handle for fundrawtransaction.
 * @param[in] txid                input utxo's transaction id.
 * @param[in] vout                input utxo's vout.
 * @param[in] amount              input utxo's amount.
 * @param[in] descriptor          the descriptor for creating locking_script.
 * @param[in] asset               utxo's unblind asset id.
 * @param[in] is_issuance         is utxo issuance input.
 * @param[in] is_blind_issuance   is utxo blind issuance input.
 * @param[in] is_pegin            is utxo pegin input.
 * @param[in] claim_script              utxo's claim script.
 * @param[in] pegin_btc_tx_size         size of pegin transaction.
 * @param[in] pegin_txoutproof_size     size of pegin txoutproof.
 * @param[in] scriptsig_template  utxo's scriptsig template.
 * @return CfdErrorCode
 */
CFDC_API int CfdAddTxInputForFundRawTx(
    void* handle, void* fund_handle, const char* txid, uint32_t vout,
    int64_t amount, const char* descriptor, const char* asset,
    bool is_issuance, bool is_blind_issuance, bool is_pegin,
    const char* claim_script, uint32_t pegin_btc_tx_size,
    uint32_t pegin_txoutproof_size, const char* scriptsig_template);

/**
 * @brief Add utxo for fundrawtransaction.
 * @param[in] handle            cfd handle.
 * @param[in] fund_handle       handle for fundrawtransaction.
 * @param[in] txid              input utxo's transaction id.
 * @param[in] vout              input utxo's vout.
 * @param[in] amount            input utxo's amount.
 * @param[in] descriptor        the descriptor for creating locking_script.
 * @param[in] asset             utxo's unblind asset id.
 * @return CfdErrorCode
 */
CFDC_API int CfdAddUtxoForFundRawTx(
    void* handle, void* fund_handle, const char* txid, uint32_t vout,
    int64_t amount, const char* descriptor, const char* asset);

/**
 * @brief Add utxo for fundrawtransaction.
 * @param[in] handle              cfd handle.
 * @param[in] fund_handle         handle for fundrawtransaction.
 * @param[in] txid                input utxo's transaction id.
 * @param[in] vout                input utxo's vout.
 * @param[in] amount              input utxo's amount.
 * @param[in] descriptor          the descriptor for creating locking_script.
 * @param[in] asset               utxo's unblind asset id.
 * @param[in] scriptsig_template  utxo's scriptsig template.
 * @return CfdErrorCode
 */
CFDC_API int CfdAddUtxoTemplateForFundRawTx(
    void* handle, void* fund_handle, const char* txid, uint32_t vout,
    int64_t amount, const char* descriptor, const char* asset,
    const char* scriptsig_template);

/**
 * @brief add fundrawtransaction's target amount.
 * @param[in] handle              cfd handle.
 * @param[in] fund_handle         handle for fundrawtransaction.
 * @param[in] asset_index         target asset index.
 * @param[in] amount              target amount.
 * @param[in] asset               target asset.
 * @param[in] reserved_address    reserved address for append txout.
 * @return CfdErrorCode
 */
CFDC_API int CfdAddTargetAmountForFundRawTx(
    void* handle, void* fund_handle, uint32_t asset_index, int64_t amount,
    const char* asset, const char* reserved_address);

/**
 * @brief set fundrawtransaction's option.
 * @param[in] handle              cfd handle.
 * @param[in] fund_handle         handle for fundrawtransaction.
 * @param[in] key                 option key. (CfdFundTxOption)
 * @param[in] int64_value         int64 value.
 * @param[in] double_value        double value.
 * @param[in] bool_value          bool value.
 * @return CfdErrorCode
 */
CFDC_API int CfdSetOptionFundRawTx(
    void* handle, void* fund_handle, int key, int64_t int64_value,
    double double_value, bool bool_value);

/**
 * @brief Finalize fundrawtransaction api call.
 * @param[in] handle                cfd handle.
 * @param[in] fund_handle           handle for fundrawtransaction.
 * @param[in] tx_hex                transaction hex.
 * @param[in] effective_fee_rate    effective fee rate.
 * @param[out] tx_fee               transaction fee ignore append txin.
 *     (not contain utxo_fee.)
 * @param[out] append_txout_count   append txout count.
 * @param[out] output_tx_hex        output fundrawtransaction.
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @return CfdErrorCode
 */
CFDC_API int CfdFinalizeFundRawTx(
    void* handle, void* fund_handle, const char* tx_hex,
    double effective_fee_rate, int64_t* tx_fee, uint32_t* append_txout_count,
    char** output_tx_hex);

/**
 * @brief Get using txout address for fundrawtransaction.
 * @param[in] handle           cfd handle.
 * @param[in] fund_handle      handle for fundrawtransaction.
 * @param[in] index            append txout count index.
 * @param[out] append_address  append address.
 *   If 'CfdFreeStringBuffer' is implemented,
 *   Call 'CfdFreeStringBuffer' after you are finished using it.
 * @return CfdErrorCode
 */
CFDC_API int CfdGetAppendTxOutFundRawTx(
    void* handle, void* fund_handle, uint32_t index, char** append_address);

/**
 * @brief Get calculate fee amount for fundrawtransaction.
 * @param[in] handle           cfd handle.
 * @param[in] fund_handle      handle for fundrawtransaction.
 * @param[out] fee_amount      calculate fee (before add dust amount)
 * @return CfdErrorCode
 */
CFDC_API int CfdGetCalculateFeeFundRawTx(
    void* handle, void* fund_handle, int64_t* fee_amount);

/**
 * @brief Free handle for  hex.
 * @param[in] handle        cfd handle.
 * @param[in] fund_handle   handle for fundrawtransaction.
 * @return CfdErrorCode
 */
CFDC_API int CfdFreeFundRawTxHandle(void* handle, void* fund_handle);

#ifdef __cplusplus
#if 0
{
#endif
}
#endif /* __cplusplus */

#endif /* CFD_INCLUDE_CFDC_CFDCAPI_TRANSACTION_H_ NOLINT */
