	.set noreorder

#include "pspstub.s"

	STUB_START "bt_ctr_driver",0x40090000,0x00070005
	STUB_FUNC  0xE6C30CBC,btCtrEnableNewConnections
	STUB_FUNC  0x0F1605A6,btCtrNewConnectionsEnabled
	STUB_FUNC  0x095EF50E,btCtrLoadControllerInfo
	STUB_FUNC  0xC0D71CDA,btCtrSetControllerInfoPolling
	STUB_FUNC  0x50597725,btCtrConnected
	STUB_FUNC  0x907D4731,btCtrDisconnectController
	STUB_FUNC  0xEF4B2B61,BTCtrTEST
	STUB_END
