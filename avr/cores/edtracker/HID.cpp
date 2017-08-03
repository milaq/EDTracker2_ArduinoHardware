/* Copyright (c) 2011, 2016, 2017 Peter Barrett, D.Howell, Micha LaQua
**
** Permission to use, copy, modify, and/or distribute this software for
** any purpose with or without fee is hereby granted, provided that the
** above copyright notice and this permission notice appear in all copies.
**
** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
** WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
** BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
** OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
** WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
** ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
** SOFTWARE.
*/

#include "USBAPI.h"

#if defined(USBCON)
#ifdef HID_ENABLED

Tracker_ Tracker;

#define LSB(_x) ((_x) & 0xFF)
#define MSB(_x) ((_x) >> 8)

#define RAWHID_USAGE_PAGE	0xFFC0
#define RAWHID_USAGE		0x0C00
#define RAWHID_TX_SIZE 64
#define RAWHID_RX_SIZE 64

#define JOYSTICK_REPORT_ID 0x01

extern const u8 _hidReportDescriptor[] PROGMEM;
const u8 _hidReportDescriptor[] = {
	/* Joystick with one dummy button and 3 axis */
	0x05, 0x01,                     // USAGE_PAGE (Generic Desktop)
	0x09, 0x04,                     // USAGE (Joystick)
	0xa1, 0x01,                     // COLLECTION (Application)
	0x85, JOYSTICK_REPORT_ID,       //   REPORT_ID

	/* 1 Dummy Button (some libraries need this) */
	0x05, 0x09,                     //   USAGE_PAGE (Button)
	0x19, 0x01,                     //   USAGE_MINIMUM (Button 1)
	0x29, 0x01,     		//   USAGE_MAXIMUM (Button 1)
	0x15, 0x00,			//   LOGICAL_MINIMUM (0)
	0x25, 0x01,			//   LOGICAL_MAXIMUM (1)
	0x75, 0x00,			//   REPORT_SIZE (0)
	0x95, 0x01,			//   REPORT_COUNT (1)
	0x81, 0x02,			//   INPUT (Data, Variable, Absolute)

	/* 3 16bit Axis */
	0x05, 0x01,			//   USAGE_PAGE (Generic Desktop)
	0xa1, 0x00,		        //   COLLECTION (Physical)
	0x09, 0x30,			//     USAGE (X)
	0x09, 0x31,			//     USAGE (Y)
	0x09, 0x32,			//     USAGE (Z)
	0x16, 0x00, 0x80,		//     LOGICAL_MINIMUM (-32768)
	0x26, 0xFF, 0x7F,		//     LOGICAL_MAXIMUM (32767)
	0x75, 0x10,			//     REPORT_SIZE (16)
	0x95, 0x03,			//     REPORT_COUNT (3)
	0x81, 0x82,			//     INPUT (Data, Variable, Absolute, Volatile)
	0xc0,				//   END_COLLECTION
	0xc0				// END_COLLECTION
};

extern const HIDDescriptor _hidInterface PROGMEM;
const HIDDescriptor _hidInterface =
{
	D_INTERFACE(HID_INTERFACE,1,3,0,0),
	D_HIDREPORT(sizeof(_hidReportDescriptor)),
	D_ENDPOINT(USB_ENDPOINT_IN (HID_ENDPOINT_INT),USB_ENDPOINT_TYPE_INTERRUPT,0x40,0x01)
};

u8 _hid_protocol = 1;
u8 _hid_idle = 1;

#define WEAK __attribute__ ((weak))

int WEAK HID_GetInterface(u8* interfaceNum)
{
	interfaceNum[0] += 1;	// uses 1
	return USB_SendControl(TRANSFER_PGM,&_hidInterface,sizeof(_hidInterface));
}

int WEAK HID_GetDescriptor(int /* i */)
{
	return USB_SendControl(TRANSFER_PGM,_hidReportDescriptor,sizeof(_hidReportDescriptor));
}

void WEAK HID_SendReport(u8 id, const void* data, int len)
{
	USB_Send(HID_TX, &id, 1);
	USB_Send(HID_TX | TRANSFER_RELEASE,data,len);
}

bool WEAK HID_Setup(Setup& setup)
{
	u8 r = setup.bRequest;
	u8 requestType = setup.bmRequestType;
	if (REQUEST_DEVICETOHOST_CLASS_INTERFACE == requestType)
	{
		if (HID_GET_REPORT == r)
		{
			return true;
		}
		if (HID_GET_PROTOCOL == r)
		{
			return true;
		}
	}

	if (REQUEST_HOSTTODEVICE_CLASS_INTERFACE == requestType)
	{
		if (HID_SET_PROTOCOL == r)
		{
			_hid_protocol = setup.wValueL;
			return true;
		}

		if (HID_SET_IDLE == r)
		{
			_hid_idle = setup.wValueL;
			return true;
		}
	}
	return false;
}

Tracker_::Tracker_()
{
}

void Tracker_::setState(TrackState_t *trackerSt)
{
	int16_t data[3];

	data[0] = trackerSt->xAxis;		// X axis
	data[1] = trackerSt->yAxis;		// Y axis
	data[2] = trackerSt->zAxis;		// Z axis

	HID_SendReport(JOYSTICK_REPORT_ID, data, 6);
}

#endif

#endif /* if defined(USBCON) */
