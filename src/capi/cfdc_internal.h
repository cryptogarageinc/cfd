// Copyright 2019 CryptoGarage
/**
 * @file cfdc_internal.h
 *
 * @brief cfd-capi内部定義ファイルです。
 *
 */
#ifndef CFD_SRC_CAPI_CFDC_INTERNAL_H_
#define CFD_SRC_CAPI_CFDC_INTERNAL_H_

#include <exception>
#include <memory>
#include <mutex>  // NOLINT
#include <vector>

#include "cfdc/cfdcapi_common.h"
#include "cfdcore/cfdcore_exception.h"

/**
 * @brief cfd名前空間
 */
namespace cfd {
/**
 * @brief capi名前空間
 */
namespace capi {

using cfd::core::CfdException;

/**
 * @brief cfd-capiハンドル情報構造体.
 */
struct CfdCapiHandleData {
  int32_t error_code;       //!< error code
  char error_message[256];  //!< error message
};

/**
 * @brief エラー情報を設定する。
 * @param[in] handle      ハンドル情報
 * @param[in] error_code  エラーコード
 * @param[in] message     エラーメッセージ
 * @return error code
 */
int SetLastError(void* handle, int error_code, const char* message);

/**
 * @brief 例外エラー情報を設定する。
 * @param[in] handle    ハンドル情報
 * @param[in] message   エラーメッセージ
 * @return error code
 */
int SetLastFatalError(void* handle, const char* message);

/**
 * @brief エラー情報を設定する。
 * @param[in] handle     ハンドル情報
 * @param[in] exception  CFD例外オブジェクト
 * @return second parameter object.
 */
const CfdException& SetLastError(void* handle, const CfdException& exception);

/**
 * @brief 例外エラー情報を設定する。
 * @param[in] handle     ハンドル情報
 * @param[in] exception  CFD例外オブジェクト
 */
void SetLastFatalError(void* handle, const std::exception& exception);

/**
 * @brief cfd-capi管理クラス。
 */
class CfdCapiManager {
 public:
  /**
   * @brief コンストラクタ。
   */
  CfdCapiManager();
  /**
   * @brief デストラクタ。
   */
  virtual ~CfdCapiManager() { FreeAllList(&handle_list_); }

  /**
   * @brief ハンドルを作成する。
   */
  void* CreateHandle(void);
  /**
   * @brief ハンドルを解放する。
   * @param[in] handle      ハンドル情報
   */
  void FreeHandle(void* handle);
  /**
   * @brief エラー情報を設定する。
   * @param[in] handle      ハンドル情報
   * @param[in] error_code  エラーコード
   * @param[in] message     エラーメッセージ
   */
  void SetLastError(void* handle, int error_code, const char* message);
  /**
   * @brief 例外エラー情報を設定する。
   * @param[in] handle    ハンドル情報
   * @param[in] message   エラーメッセージ
   */
  void SetLastFatalError(void* handle, const char* message);
  /**
   * @brief エラー情報を取得する。
   * @param[in] handle      ハンドル情報
   * @return 例外情報
   */
  CfdException GetLastError(void* handle);

 protected:
  std::vector<CfdCapiHandleData*> handle_list_;  ///< ハンドル一覧
  std::mutex mutex_;  ///< 排他制御用オブジェクト

  /**
   * @brief ハンドル一覧の解放を行う。
   * @param[in,out] list  handle list
   */
  static void FreeAllList(std::vector<CfdCapiHandleData*>* list);
};

}  // namespace capi
}  // namespace cfd

#endif  // CFD_SRC_CAPI_CFDC_INTERNAL_H_
