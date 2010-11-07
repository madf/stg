#include "user_property.h"

//-----------------------------------------------------------------------------
USER_PROPERTIES::USER_PROPERTIES(const SETTINGS * s)
:
cash            (stat.cash,             "cash",             false, true, GetStgLogger(), s),
up              (stat.up,               "upload",           false, true, GetStgLogger(), s),
down            (stat.down,             "download",         false, true, GetStgLogger(), s),
lastCashAdd     (stat.lastCashAdd,      "lastCashAdd",      false, true, GetStgLogger(), s),
passiveTime     (stat.passiveTime,      "passiveTime",      false, true, GetStgLogger(), s),
lastCashAddTime (stat.lastCashAddTime,  "lastCashAddTime",  false, true, GetStgLogger(), s),
freeMb          (stat.freeMb,           "freeMb",           false, true, GetStgLogger(), s),
lastActivityTime(stat.lastActivityTime, "lastActivityTime", false, true, GetStgLogger(), s),


password    (conf.password,     "password",     true,  false, GetStgLogger(), s),
passive     (conf.passive,      "passive",      false, false, GetStgLogger(), s),
disabled    (conf.disabled,     "disabled",     false, false, GetStgLogger(), s),
disabledDetailStat(conf.disabledDetailStat,"DisabledDetailStat",false,false,GetStgLogger(), s),
alwaysOnline(conf.alwaysOnline, "alwaysOnline", false, false, GetStgLogger(), s),
tariffName  (conf.tariffName,   "tariff",       false, false, GetStgLogger(), s),
nextTariff  (conf.nextTariff,   "new tariff",   false, false, GetStgLogger(), s),
address     (conf.address,      "address",      false, false, GetStgLogger(), s),
note        (conf.note,         "note",         false, false, GetStgLogger(), s),
group       (conf.group,        "group",        false, false, GetStgLogger(), s),
email       (conf.email,        "email",        false, false, GetStgLogger(), s),
phone       (conf.phone,        "phone",        false, false, GetStgLogger(), s),
realName    (conf.realName,     "realName",     false, false, GetStgLogger(), s),
credit      (conf.credit,       "credit",       false, false, GetStgLogger(), s),
creditExpire(conf.creditExpire, "creditExpire", false, false, GetStgLogger(), s),
ips         (conf.ips,          "IP",           false, false, GetStgLogger(), s),
userdata0   (conf.userdata[0],  "userdata0",    false, false, GetStgLogger(), s),
userdata1   (conf.userdata[1],  "userdata1",    false, false, GetStgLogger(), s),
userdata2   (conf.userdata[2],  "userdata2",    false, false, GetStgLogger(), s),
userdata3   (conf.userdata[3],  "userdata3",    false, false, GetStgLogger(), s),
userdata4   (conf.userdata[4],  "userdata4",    false, false, GetStgLogger(), s),
userdata5   (conf.userdata[5],  "userdata5",    false, false, GetStgLogger(), s),
userdata6   (conf.userdata[6],  "userdata6",    false, false, GetStgLogger(), s),
userdata7   (conf.userdata[7],  "userdata7",    false, false, GetStgLogger(), s),
userdata8   (conf.userdata[8],  "userdata8",    false, false, GetStgLogger(), s),
userdata9   (conf.userdata[9],  "userdata9",    false, false, GetStgLogger(), s)
{

}
//-----------------------------------------------------------------------------

