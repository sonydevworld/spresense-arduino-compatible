/*
 *  LTECore.cpp - LTECore implementation file for Spresense Arduino
 *  Copyright 2019 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file LTECore.cpp
 *
 * @author Sony Semiconductor Solutions Corporation
 *
 * @brief LTE Library for Spresense Arduino.
 *
 * @details Internal use class for Spresense Arduino LTE library.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <Print.h>
#include <stdint.h>
#include <errno.h>

#include <arch/board/cxd56_alt1250.h>

#include <lte/lte_api.h>
#include <LTECore.h>

/****************************************************************************
 * Private Data
 ****************************************************************************/

LTECore* LTECore::_instance = NULL;

static pthread_t g_recoveryThreadID = -1;

/****************************************************************************
 * Private function implementation.
 ****************************************************************************/

static void* recoveryThread(void* arg)
{
  theLTECore.recovery();
  LTEDBG("theLTECore.recovery() complete.\n");
  g_recoveryThreadID = -1;
  pthread_exit(0);
}

/* Callback for Asynchronous SDK function. */

extern "C"
{

static void modemRestartCallback(uint32_t reason)
{
  LTEDBG("Modem restart : %lu\n", reason);

  switch (reason) {
    case LTE_RESTART_MODEM_INITIATED:

      /* When the modem restarts itself,
       * process is executed to restore the state before the restart.
       */
      if (g_recoveryThreadID < 0) {
          if (0 == pthread_create(&g_recoveryThreadID, NULL, recoveryThread, NULL)) {
            LTEDBG("Recovery thread create.\n");
            pthread_detach(g_recoveryThreadID);
          } else {
            LTEERR("Recovery thread creation failure.\n");
            theLTECore.setStatus(LTE_ERROR);
          }
      } else {
        LTEERR("Modem restart during recovery.\n");
        theLTECore.setStatus(LTE_ERROR);
      }
      break;
    case LTE_RESTART_USER_INITIATED:
      theLTECore.signalModemReset();
      break;
    default:
      LTEERR("Illegal reason : %lu\n", reason);
      break;
  }
}

static void activatePDNCallback(uint32_t result, lte_pdn_t *pdn)
{
  switch (result)
  {
    case LTE_RESULT_ERROR:
      LTEERR("Attach Error.\n");
      theLTECore.setStatus(LTE_ERROR);
      theLTECore.printErrorInfo();
      return;
    case LTE_RESULT_CANCEL:
      LTEDBG("Attach cancel.\n");
      if (LTE_CONNECTING == theLTECore.getStatus()) {
        theLTECore.setStatus(LTE_SEARCHING);
      }
      return;
    default:
      break;
  }

  if ((pdn->apn_type & LTE_APN_TYPE_IMS) != 0) {
    LTEDBG("Successful IMS attach.\n");
    theLTECore.setStatus(LTE_SEARCHING);
  } else if ((pdn->apn_type & LTE_APN_TYPE_DEFAULT) != 0) {
    LTEDBG("Successful PDN attach.\n");
    lte_set_report_netinfo(NULL);
    theLTECore.setSessionID(pdn->session_id);
    theLTECore.setStatus(LTE_READY);
  }
}

static void reportNetinfoCallback(lte_netinfo_t *info)
{
  LTEDBG("Report netinfo stat : %d\n", info->nw_stat);

  if (info->nw_stat == LTE_NETSTAT_REG_DENIED) {
    LTEDBG("Report netinfo err_type : %d\n", info->nw_err.err_type);

    /* In case of REJECT, cancel the activtePDN process. */
    if (info->nw_err.err_type == LTE_NETERR_REJECT) {
      LTEERR("Rejected from the network.\n");
      theLTECore.setStatus(LTE_ERROR);
      lte_activate_pdn_cancel();
    }
  }
}

}

/****************************************************************************
 * LTECore implementation.
 ****************************************************************************/
LTECore::LTECore():_networkStatus(LTE_OFF), _sessionID(0)
{
  memset(_modemPinCode, 0, LTE_NET_PINCODE_MAXLEN);
  memset(_apn.name, 0, LTE_NET_APN_MAXLEN);
  memset(_apn.userName, 0, LTE_NET_USER_MAXLEN);
  memset(_apn.password, 0, LTE_NET_PASSWORD_MAXLEN);
  _apn.authType = LTE_NET_AUTHTYPE_CHAP;
  _apn.ipType   = LTE_NET_IPTYPE_V4V6;
}

LTECore::~LTECore()
{
  shutdown();
}

LTECore LTECore::getInstance()
{
  if (LTECore::_instance == NULL) {
    LTECore::_instance = new LTECore();
  }
  return *LTECore::_instance;
}

LTEModemStatus LTECore::begin(bool restart)
{
  int result = 0;

  if (restart) {
    shutdown();
  }

  result = board_alt1250_initialize("/dev/alt1250");
  if (result < 0) {
    LTEDBG("Fatal error couldn't initialize modem driver\n");
    goto errout;
  }

  result = lte_initialize();
  if (result < 0) {
    if (-EALREADY == result) {
      LTEDBG("Already powered on.\n");
      return getStatus();
    } else {
      LTEERR("lte_initialize result error : %d\n", result);
      goto errout;
    }
  }

  result = lte_set_report_restart(modemRestartCallback);
  if (result < 0) {
    LTEERR("lte_set_report_restart result error : %d\n", result);
    goto errout;
  }

  pthread_cond_init(&(_resetCondition.cond), NULL);
  pthread_mutex_init(&(_resetCondition.mutex), NULL);

  pthread_mutex_lock(&(_resetCondition.mutex));

  result = lte_power_on();
  if (result < 0) {
    LTEERR("lte_power_on result error : %d\n", result);
    goto errout;
  }

  /* Wait for modemRestartCallback. */

  pthread_cond_wait(&(_resetCondition.cond), &(_resetCondition.mutex));
  pthread_mutex_unlock(&(_resetCondition.mutex));

  setStatus(LTE_IDLE);

  LTEDBG("Successful modem poweron.\n");

  return getStatus();

errout:
  shutdown();
  setStatus(LTE_ERROR);
  return getStatus();
}

void LTECore::shutdown()
{
  if (LTE_OFF == getStatus()) {
    return;
  }

  setStatus(LTE_OFF);
  lte_finalize();

  /* Clear string. */

  _modemPinCode[0] = '\0';
  _apn.name[0]     = '\0';
  _apn.userName[0] = '\0';
  _apn.password[0] = '\0';

  _apn.authType = LTE_NET_AUTHTYPE_CHAP;
  _apn.ipType   = LTE_NET_IPTYPE_V4V6;

  setSessionID(0);

  /* Release any task waiting for cond. */

  signalModemReset();

  pthread_cond_destroy(&(_resetCondition.cond));
  pthread_mutex_destroy(&(_resetCondition.mutex));

}

LTEModemStatus LTECore::startSearchNetwork(char* pinCode, bool synchronous)
{
  int            result        = 0;
  bool           imsCapability = false;
  LTEModemStatus status        = LTE_SEARCHING;

  if (pinCode && (0 != strnlen(pinCode, LTE_NET_PINCODE_MAXLEN - 1))) {

    uint8_t simStatus    = 0;
    uint8_t attemptsleft = 0;

    result = lte_enter_pin_sync(pinCode, NULL, &simStatus, &attemptsleft);
    if (result < 0) {
      LTEERR("lte_enter_pin_sync result error : %d\n", result);
      LTEERR("simStatus : %d\n", simStatus);
      LTEERR("attemptsleft : %d\n", attemptsleft);
      if (-EPROTO == result) {
        printErrorInfo();
      }
      goto errout;
    }

    LTEDBG("Successful unlock PIN code: %s\n", pinCode);

    /* Copy to privte member */

    memset(_modemPinCode, 0, LTE_NET_PINCODE_MAXLEN);
    strncpy(_modemPinCode, pinCode, strnlen(pinCode, LTE_NET_PINCODE_MAXLEN - 1));

  }

  result = lte_radio_on_sync();
  if (result < 0) {
    LTEERR("lte_radio_on_sync result error : %d\n", result);
    if (-EPROTO == result) {
      printErrorInfo();
    }
    goto errout;
  }

  LTEDBG("Successful start searching.\n");

  result = lte_get_imscap_sync(&imsCapability);
  if (result < 0) {
    LTEERR("lte_get_imscap_sync result error : %d", result);
    if (-EPROTO == result) {
      printErrorInfo();
    }
    goto errout;
  }

  LTEDBG("Successful get IMS capability : %s \n", (imsCapability ? "TRUE" : "FALSE"));

  /* Automatically connect to IMS when IMS is valid.
   * Synchronous parameter has no meaning when IMS is invalid. 
   */
  if (imsCapability) {

    lte_apn_setting_t imsSetting;

    memset(&imsSetting, 0, sizeof(lte_apn_setting_t));

    /* For IMS connections, APN name does not make sense. */
    
    imsSetting.apn = const_cast<char*>("");
    imsSetting.apn_type = LTE_APN_TYPE_IA | LTE_APN_TYPE_IMS;
    imsSetting.auth_type = LTE_NET_AUTHTYPE_NONE;

    if (synchronous) {
      lte_pdn_t imsResult;

      result = lte_activate_pdn_sync(&imsSetting, &imsResult);
      if (result < 0) {
        LTEERR("lte_activate_pdn_sync result error : %d\n", result);
        if (-EPROTO == result) {
          printErrorInfo();
        }
        goto errout;

      }

      LTEDBG("Successful IMS connect.\n");
      status = LTE_SEARCHING;
    } else {
      result = lte_activate_pdn(&imsSetting, activatePDNCallback);
      if (result < 0) {
        LTEERR("lte_activate_pdn result error : %d\n", result);
        goto errout;
      }
      status = LTE_CONNECTING;
    }
  }

  return status;

errout:
  memset(_modemPinCode, 0, LTE_NET_PINCODE_MAXLEN);
  setStatus(LTE_ERROR);
  return LTE_ERROR;
}

LTEModemStatus LTECore::connectNetwork(const char *apn, const char *userName, const char *password, LTENetworkAuthType authType, LTENetworkIPType ipType, bool synchronous, bool cancelable)
{
  int               result     = 0;
  LTEModemStatus    status     = LTE_SEARCHING;
  lte_apn_setting_t apnSetting = {const_cast<char*>(apn),
                                  ipType,
                                  authType,
                                  LTE_APN_TYPE_IA | LTE_APN_TYPE_DEFAULT,
                                  const_cast<char*>(userName),
                                  const_cast<char*>(password)
                                  };

  if (!apnSetting.apn || (0 == strlen(apnSetting.apn))) {
    LTEERR("Invalid APN name.\n");
    setStatus(LTE_ERROR);
    return LTE_ERROR;
  }

  /* If the user name and password are NULL or 0 length,
   * there is no authentication.
   */

  if (!apnSetting.user_name || (0 == strlen(apnSetting.user_name)) ||
      !apnSetting.password || (0 == strlen(apnSetting.password))) {
    apnSetting.user_name = NULL;
    apnSetting.password = NULL;
    apnSetting.auth_type = LTE_NET_AUTHTYPE_NONE;
  }

  bool imsCapability = LTE_DISABLE;

  result = lte_get_imscap_sync(&imsCapability);
  if (result < 0) {
    LTEERR("lte_get_imscap_sync result error : %d\n", result);
    if (-EPROTO == result) {
      printErrorInfo();
    }
    goto errout;
  }

  if (imsCapability) {
    apnSetting.apn_type = LTE_APN_TYPE_DEFAULT;
  } else {
    apnSetting.apn_type = LTE_APN_TYPE_IA | LTE_APN_TYPE_DEFAULT;
  }

  if (cancelable) {
    result = lte_set_report_netinfo(reportNetinfoCallback);
    if (result < 0) {
      LTEERR("lte_set_report_netinfo result error : %d\n", result);
      goto errout;
    }
  }

  if (synchronous) {
    lte_pdn_t resultPDN;

    result = lte_activate_pdn_sync(&apnSetting, &resultPDN);
    if (result < 0) {
      LTEERR("lte_activate_pdn_sync result error : %d\n", result);
      if (-EPROTO == result) {
        printErrorInfo();
      }

      goto errout;
    }

    setSessionID(resultPDN.session_id);

    if (cancelable) {
      result = lte_set_report_netinfo(NULL);
      if (result < 0) {
        LTEERR("lte_set_report_netinfo result error : %d\n", result);
        goto errout;
      }
    }

    status = LTE_READY;

    LTEDBG("Successful PDN attach.\n");
  } else {
    result = lte_activate_pdn(&apnSetting, activatePDNCallback);
    if (result < 0) {
      LTEERR("lte_activate_pdn result error : %d\n", result);
      goto errout;
    }
    status = LTE_CONNECTING;
  }

  /* Copy to private members */

  strncpy(_apn.name, apn, strnlen(apn, LTE_NET_APN_MAXLEN - 1));
  _apn.name[strnlen(apn, LTE_NET_APN_MAXLEN - 1)] = '\0';

  _apn.authType = static_cast<LTENetworkAuthType>(apnSetting.auth_type);
  _apn.ipType = ipType;

  if (_apn.authType != LTE_NET_AUTHTYPE_NONE) {
    strncpy(_apn.userName, userName, strnlen(userName, LTE_NET_USER_MAXLEN - 1));
    _apn.userName[strnlen(userName, LTE_NET_USER_MAXLEN - 1)] = '\0';
    strncpy(_apn.password, password, strnlen(password, LTE_NET_PASSWORD_MAXLEN - 1));
    _apn.password[strnlen(userName, LTE_NET_PASSWORD_MAXLEN - 1)] = '\0';
  }

  return status;

errout:
  _apn.name[0]     = '\0';
  _apn.userName[0] = '\0';
  _apn.password[0] = '\0';
  _apn.authType = LTE_NET_AUTHTYPE_CHAP;
  _apn.ipType   = LTE_NET_IPTYPE_V4V6;

  setStatus(LTE_ERROR);
  return LTE_ERROR;
}

LTEModemStatus LTECore::disconnectNetwork()
{
  int result = 0;

  if (0 < _sessionID) {
    result = lte_deactivate_pdn_sync(_sessionID);
    if (result < 0) {  
      LTEERR("lte_deactivate_pdn_sync result error : %d\n", result);
      if (-EPROTO == result) {
        printErrorInfo();
      }
      goto errout;
    }

    LTEDBG("Successful PDN detach.\n");

    /* Clears the APN information held in the class. */

    _apn.name[0]     = '\0';
    _apn.userName[0] = '\0';
    _apn.password[0] = '\0';
    _apn.authType    = LTE_NET_AUTHTYPE_CHAP;
    _apn.ipType      = LTE_NET_IPTYPE_V4V6;

    setSessionID(0);

    setStatus(LTE_SEARCHING);

  } else {

    /* Attach cancel is executed when called without a session ID assigned. */

    LTEDBG("Send PDN attach cancel command.\n");
    result = lte_activate_pdn_cancel();
    if (result < 0) {  
      LTEERR("lte_activate_pdn_cancel result error : %d\n", result);
      goto errout;
    }

    /* Poll every 100ms until activate_pdn returns. */

    while(LTE_CONNECTING == getStatus()) {
      usleep(100 * 1000);
    }
  }

  return getStatus();
errout:
  setStatus(LTE_ERROR);
  return getStatus();
}

void LTECore::signalModemReset()
{
  pthread_mutex_lock(&(_resetCondition.mutex));
  pthread_cond_signal(&(_resetCondition.cond));
  pthread_mutex_unlock(&(_resetCondition.mutex));
}

void LTECore::printErrorInfo()
{
  int            result  = 0;
  lte_errinfo_t  errinfo;

  result = lte_get_errinfo(&errinfo);
  if (result == 0) {
    if (0 != (errinfo.err_indicator & LTE_ERR_INDICATOR_ERRCODE)) {
      LTEERR("Errorinfo errcode : %ld\n", errinfo.err_result_code);
    }
    if (0 != (errinfo.err_indicator & LTE_ERR_INDICATOR_ERRNO)) {
      LTEERR("Errorinfo errno : %ld\n", errinfo.err_no);
    }
    if (0 != (errinfo.err_indicator & LTE_ERR_INDICATOR_ERRSTR)) {
      LTEERR("Errorinfo errstr : %s\n", errinfo.err_string);
    }
  } else {
    LTEERR("lte_get_errinfo result error : %d\n", result);
  }
}

void LTECore::recovery()
{
  LTEModemStatus newStat;
  LTEModemStatus oldStat = getStatus();

  LTEDBG("Recovery Thread : oldStat = %d\n", oldStat);

  switch (oldStat) {
    case LTE_SEARCHING:
    case LTE_CONNECTING:
    case LTE_READY:
      newStat = startSearchNetwork(_modemPinCode, true);
      if ((LTE_SEARCHING == newStat) &&
          (0 != strnlen(_apn.name, LTE_NET_APN_MAXLEN - 1))) {
        newStat = connectNetwork(_apn.name, _apn.userName, _apn.password,
                                 _apn.authType, _apn.ipType, true, false);
      }

      setStatus(newStat);
      if (LTE_ERROR != newStat) {
        LTEDBG("Recovery Complete  : %d\n", newStat);
      } else {
        LTEERR("Recovery Failed  : %d\n", newStat);
      }
      break; 
    default:
      break;
  }
}

/** Global instance */
LTECore theLTECore = LTECore::getInstance();
