	.set noreorder

#include "pspstub.s"

	STUB_START "bt_ctr_driver",0x40090000,0x00030005
	STUB_FUNC  0xD44EB261,BtCtrDriverEnableNewConnections
	STUB_FUNC  0x8E69F5D8,BtCtrDriverNewConnectionsEnabled
	STUB_FUNC  0x4A6AB5B0,BtCtrDriverLoadControllerInfo
	STUB_END
