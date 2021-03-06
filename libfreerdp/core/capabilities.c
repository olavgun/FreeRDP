/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * RDP Capability Sets
 *
 * Copyright 2011 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "capabilities.h"

/*
static const char* const CAPSET_TYPE_STRINGS[] =
{
		"Unknown",
		"General",
		"Bitmap",
		"Order",
		"Bitmap Cache",
		"Control",
		"Unknown",
		"Window Activation",
		"Pointer",
		"Share",
		"Color Cache",
		"Unknown",
		"Sound",
		"Input",
		"Font",
		"Brush",
		"Glyph Cache",
		"Offscreen Bitmap Cache",
		"Bitmap Cache Host Support",
		"Bitmap Cache v2",
		"Virtual Channel",
		"DrawNineGrid Cache",
		"Draw GDI+ Cache",
		"Remote Programs",
		"Window List",
		"Desktop Composition",
		"Multifragment Update",
		"Large Pointer",
		"Surface Commands",
		"Bitmap Codecs",
		"Frame Acknowledge"
};
*/

/* CODEC_GUID_REMOTEFX 0x76772F12BD724463AFB3B73C9C6F7886 */
#define CODEC_GUID_REMOTEFX "\x12\x2F\x77\x76\x72\xBD\x63\x44\xAF\xB3\xB7\x3C\x9C\x6F\x78\x86"

/* CODEC_GUID_NSCODEC  0xCA8D1BB9000F154F589FAE2D1A87E2D6 */
#define CODEC_GUID_NSCODEC "\xb9\x1b\x8d\xca\x0f\x00\x4f\x15\x58\x9f\xae\x2d\x1a\x87\xe2\xd6"

/* CODEC_GUID_JPEG 0x430C9EED1BAF4CE6869ACB8B37B66237*/
#define CODEC_GUID_JPEG "\xE6\x4C\xAF\x1B\xED\x9E\x0C\x43\x86\x9A\xCB\x8B\x37\xB6\x62\x37"

void rdp_read_capability_set_header(STREAM* s, UINT16* length, UINT16* type)
{
	stream_read_UINT16(s, *type); /* capabilitySetType */
	stream_read_UINT16(s, *length); /* lengthCapability */
}

void rdp_write_capability_set_header(STREAM* s, UINT16 length, UINT16 type)
{
	stream_write_UINT16(s, type); /* capabilitySetType */
	stream_write_UINT16(s, length); /* lengthCapability */
}

BYTE* rdp_capability_set_start(STREAM* s)
{
	BYTE* header;

	stream_get_mark(s, header);
	stream_write_zero(s, CAPSET_HEADER_LENGTH);

	return header;
}

void rdp_capability_set_finish(STREAM* s, BYTE* header, UINT16 type)
{
	UINT16 length;
	BYTE* footer;

	footer = s->p;
	length = footer - header;
	stream_set_mark(s, header);

	rdp_write_capability_set_header(s, length, type);
	stream_set_mark(s, footer);
}

/**
 * Read general capability set.\n
 * @msdn{cc240549}
 * @param s stream
 * @param settings settings
 */

void rdp_read_general_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	UINT16 extraFlags;
	BYTE refreshRectSupport;
	BYTE suppressOutputSupport;

	if (settings->ServerMode)
	{
		stream_read_UINT16(s, settings->OsMajorType); /* osMajorType (2 bytes) */
		stream_read_UINT16(s, settings->OsMinorType); /* osMinorType (2 bytes) */
	}
	else
	{
		stream_seek_UINT16(s); /* osMajorType (2 bytes) */
		stream_seek_UINT16(s); /* osMinorType (2 bytes) */
	}
	stream_seek_UINT16(s); /* protocolVersion (2 bytes) */
	stream_seek_UINT16(s); /* pad2OctetsA (2 bytes) */
	stream_seek_UINT16(s); /* generalCompressionTypes (2 bytes) */
	stream_read_UINT16(s, extraFlags); /* extraFlags (2 bytes) */
	stream_seek_UINT16(s); /* updateCapabilityFlag (2 bytes) */
	stream_seek_UINT16(s); /* remoteUnshareFlag (2 bytes) */
	stream_seek_UINT16(s); /* generalCompressionLevel (2 bytes) */
	stream_read_BYTE(s, refreshRectSupport); /* refreshRectSupport (1 byte) */
	stream_read_BYTE(s, suppressOutputSupport); /* suppressOutputSupport (1 byte) */

	if (!(extraFlags & FASTPATH_OUTPUT_SUPPORTED))
		settings->FastPathOutput = FALSE;

	if (refreshRectSupport == FALSE)
		settings->RefreshRect = FALSE;

	if (suppressOutputSupport == FALSE)
		settings->SuppressOutput = FALSE;
}

/**
 * Write general capability set.\n
 * @msdn{cc240549}
 * @param s stream
 * @param settings settings
 */

void rdp_write_general_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT16 extraFlags;

	header = rdp_capability_set_start(s);

	extraFlags = LONG_CREDENTIALS_SUPPORTED | NO_BITMAP_COMPRESSION_HDR;

	if (settings->AutoReconnectionEnabled)
		extraFlags |= AUTORECONNECT_SUPPORTED;

	if (settings->FastPathOutput)
		extraFlags |= FASTPATH_OUTPUT_SUPPORTED;

	if (settings->SaltedChecksum)
		extraFlags |= ENC_SALTED_CHECKSUM;

	stream_write_UINT16(s, settings->OsMajorType); /* osMajorType (2 bytes) */
	stream_write_UINT16(s, settings->OsMinorType); /* osMinorType (2 bytes) */
	stream_write_UINT16(s, CAPS_PROTOCOL_VERSION); /* protocolVersion (2 bytes) */
	stream_write_UINT16(s, 0); /* pad2OctetsA (2 bytes) */
	stream_write_UINT16(s, 0); /* generalCompressionTypes (2 bytes) */
	stream_write_UINT16(s, extraFlags); /* extraFlags (2 bytes) */
	stream_write_UINT16(s, 0); /* updateCapabilityFlag (2 bytes) */
	stream_write_UINT16(s, 0); /* remoteUnshareFlag (2 bytes) */
	stream_write_UINT16(s, 0); /* generalCompressionLevel (2 bytes) */
	stream_write_BYTE(s, settings->RefreshRect); /* refreshRectSupport (1 byte) */
	stream_write_BYTE(s, settings->SuppressOutput); /* suppressOutputSupport (1 byte) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_GENERAL);
}

/**
 * Read bitmap capability set.\n
 * @msdn{cc240554}
 * @param s stream
 * @param settings settings
 */

void rdp_read_bitmap_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	BYTE drawingFlags;
	UINT16 desktopWidth;
	UINT16 desktopHeight;
	UINT16 desktopResizeFlag;
	UINT16 preferredBitsPerPixel;

	stream_read_UINT16(s, preferredBitsPerPixel); /* preferredBitsPerPixel (2 bytes) */
	stream_seek_UINT16(s); /* receive1BitPerPixel (2 bytes) */
	stream_seek_UINT16(s); /* receive4BitsPerPixel (2 bytes) */
	stream_seek_UINT16(s); /* receive8BitsPerPixel (2 bytes) */
	stream_read_UINT16(s, desktopWidth); /* desktopWidth (2 bytes) */
	stream_read_UINT16(s, desktopHeight); /* desktopHeight (2 bytes) */
	stream_seek_UINT16(s); /* pad2Octets (2 bytes) */
	stream_read_UINT16(s, desktopResizeFlag); /* desktopResizeFlag (2 bytes) */
	stream_seek_UINT16(s); /* bitmapCompressionFlag (2 bytes) */
	stream_seek_BYTE(s); /* highColorFlags (1 byte) */
	stream_read_BYTE(s, drawingFlags); /* drawingFlags (1 byte) */
	stream_seek_UINT16(s); /* multipleRectangleSupport (2 bytes) */
	stream_seek_UINT16(s); /* pad2OctetsB (2 bytes) */

	if (!settings->ServerMode && preferredBitsPerPixel != settings->ColorDepth)
	{
		/* The client must respect the actual color depth used by the server */
		settings->ColorDepth = preferredBitsPerPixel;
	}

	if (desktopResizeFlag == FALSE)
		settings->DesktopResize = FALSE;

	if (!settings->ServerMode && settings->DesktopResize)
	{
		/* The server may request a different desktop size during Deactivation-Reactivation sequence */
		settings->DesktopWidth = desktopWidth;
		settings->DesktopHeight = desktopHeight;
	}
}

/**
 * Write bitmap capability set.\n
 * @msdn{cc240554}
 * @param s stream
 * @param settings settings
 */

void rdp_write_bitmap_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	BYTE drawingFlags = 0;
	UINT16 desktopResizeFlag;
	UINT16 preferredBitsPerPixel;

	header = rdp_capability_set_start(s);

	drawingFlags |= DRAW_ALLOW_SKIP_ALPHA;

	if (settings->RdpVersion > 5)
		preferredBitsPerPixel = settings->ColorDepth;
	else
		preferredBitsPerPixel = 8;

	desktopResizeFlag = settings->DesktopResize;

	stream_write_UINT16(s, preferredBitsPerPixel); /* preferredBitsPerPixel (2 bytes) */
	stream_write_UINT16(s, 1); /* receive1BitPerPixel (2 bytes) */
	stream_write_UINT16(s, 1); /* receive4BitsPerPixel (2 bytes) */
	stream_write_UINT16(s, 1); /* receive8BitsPerPixel (2 bytes) */
	stream_write_UINT16(s, settings->DesktopWidth); /* desktopWidth (2 bytes) */
	stream_write_UINT16(s, settings->DesktopHeight); /* desktopHeight (2 bytes) */
	stream_write_UINT16(s, 0); /* pad2Octets (2 bytes) */
	stream_write_UINT16(s, desktopResizeFlag); /* desktopResizeFlag (2 bytes) */
	stream_write_UINT16(s, 1); /* bitmapCompressionFlag (2 bytes) */
	stream_write_BYTE(s, 0); /* highColorFlags (1 byte) */
	stream_write_BYTE(s, drawingFlags); /* drawingFlags (1 byte) */
	stream_write_UINT16(s, 1); /* multipleRectangleSupport (2 bytes) */
	stream_write_UINT16(s, 0); /* pad2OctetsB (2 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_BITMAP);
}

/**
 * Read order capability set.\n
 * @msdn{cc240556}
 * @param s stream
 * @param settings settings
 */

void rdp_read_order_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	int i;
	UINT16 orderFlags;
	BYTE orderSupport[32];
	UINT16 orderSupportExFlags;

	stream_seek(s, 16); /* terminalDescriptor (16 bytes) */
	stream_seek_UINT32(s); /* pad4OctetsA (4 bytes) */
	stream_seek_UINT16(s); /* desktopSaveXGranularity (2 bytes) */
	stream_seek_UINT16(s); /* desktopSaveYGranularity (2 bytes) */
	stream_seek_UINT16(s); /* pad2OctetsA (2 bytes) */
	stream_seek_UINT16(s); /* maximumOrderLevel (2 bytes) */
	stream_seek_UINT16(s); /* numberFonts (2 bytes) */
	stream_read_UINT16(s, orderFlags); /* orderFlags (2 bytes) */
	stream_read(s, orderSupport, 32); /* orderSupport (32 bytes) */
	stream_seek_UINT16(s); /* textFlags (2 bytes) */
	stream_read_UINT16(s, orderSupportExFlags); /* orderSupportExFlags (2 bytes) */
	stream_seek_UINT32(s); /* pad4OctetsB (4 bytes) */
	stream_seek_UINT32(s); /* desktopSaveSize (4 bytes) */
	stream_seek_UINT16(s); /* pad2OctetsC (2 bytes) */
	stream_seek_UINT16(s); /* pad2OctetsD (2 bytes) */
	stream_seek_UINT16(s); /* textANSICodePage (2 bytes) */
	stream_seek_UINT16(s); /* pad2OctetsE (2 bytes) */

	for (i = 0; i < 32; i++)
	{
		if (orderSupport[i] == FALSE)
			settings->OrderSupport[i] = FALSE;
	}
}

/**
 * Write order capability set.\n
 * @msdn{cc240556}
 * @param s stream
 * @param settings settings
 */

void rdp_write_order_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT16 orderFlags;
	UINT16 orderSupportExFlags;
	UINT16 textANSICodePage;

	header = rdp_capability_set_start(s);

	/* see [MSDN-CP]: http://msdn.microsoft.com/en-us/library/dd317756 */
	textANSICodePage = 65001; /* Unicode (UTF-8) */

	orderSupportExFlags = 0;
	orderFlags = NEGOTIATE_ORDER_SUPPORT | ZERO_BOUNDS_DELTA_SUPPORT | COLOR_INDEX_SUPPORT;

	if (settings->BitmapCacheV3Enabled)
	{
		orderSupportExFlags |= CACHE_BITMAP_V3_SUPPORT;
		orderFlags |= ORDER_FLAGS_EXTRA_SUPPORT;
	}

	if (settings->FrameMarkerCommandEnabled)
	{
		orderSupportExFlags |= ALTSEC_FRAME_MARKER_SUPPORT;
		orderFlags |= ORDER_FLAGS_EXTRA_SUPPORT;
	}

	stream_write_zero(s, 16); /* terminalDescriptor (16 bytes) */
	stream_write_UINT32(s, 0); /* pad4OctetsA (4 bytes) */
	stream_write_UINT16(s, 1); /* desktopSaveXGranularity (2 bytes) */
	stream_write_UINT16(s, 20); /* desktopSaveYGranularity (2 bytes) */
	stream_write_UINT16(s, 0); /* pad2OctetsA (2 bytes) */
	stream_write_UINT16(s, 1); /* maximumOrderLevel (2 bytes) */
	stream_write_UINT16(s, 0); /* numberFonts (2 bytes) */
	stream_write_UINT16(s, orderFlags); /* orderFlags (2 bytes) */
	stream_write(s, settings->OrderSupport, 32); /* orderSupport (32 bytes) */
	stream_write_UINT16(s, 0); /* textFlags (2 bytes) */
	stream_write_UINT16(s, orderSupportExFlags); /* orderSupportExFlags (2 bytes) */
	stream_write_UINT32(s, 0); /* pad4OctetsB (4 bytes) */
	stream_write_UINT32(s, 230400); /* desktopSaveSize (4 bytes) */
	stream_write_UINT16(s, 0); /* pad2OctetsC (2 bytes) */
	stream_write_UINT16(s, 0); /* pad2OctetsD (2 bytes) */
	stream_write_UINT16(s, 0); /* textANSICodePage (2 bytes) */
	stream_write_UINT16(s, 0); /* pad2OctetsE (2 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_ORDER);
}

/**
 * Read bitmap cache capability set.\n
 * @msdn{cc240559}
 * @param s stream
 * @param settings settings
 */

void rdp_read_bitmap_cache_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	stream_seek_UINT32(s); /* pad1 (4 bytes) */
	stream_seek_UINT32(s); /* pad2 (4 bytes) */
	stream_seek_UINT32(s); /* pad3 (4 bytes) */
	stream_seek_UINT32(s); /* pad4 (4 bytes) */
	stream_seek_UINT32(s); /* pad5 (4 bytes) */
	stream_seek_UINT32(s); /* pad6 (4 bytes) */
	stream_seek_UINT16(s); /* Cache0Entries (2 bytes) */
	stream_seek_UINT16(s); /* Cache0MaximumCellSize (2 bytes) */
	stream_seek_UINT16(s); /* Cache1Entries (2 bytes) */
	stream_seek_UINT16(s); /* Cache1MaximumCellSize (2 bytes) */
	stream_seek_UINT16(s); /* Cache2Entries (2 bytes) */
	stream_seek_UINT16(s); /* Cache2MaximumCellSize (2 bytes) */
}

/**
 * Write bitmap cache capability set.\n
 * @msdn{cc240559}
 * @param s stream
 * @param settings settings
 */

void rdp_write_bitmap_cache_capability_set(STREAM* s, rdpSettings* settings)
{
	int bpp;
	UINT16 size;
	BYTE* header;

	header = rdp_capability_set_start(s);

	bpp = (settings->ColorDepth + 7) / 8;

	stream_write_UINT32(s, 0); /* pad1 (4 bytes) */
	stream_write_UINT32(s, 0); /* pad2 (4 bytes) */
	stream_write_UINT32(s, 0); /* pad3 (4 bytes) */
	stream_write_UINT32(s, 0); /* pad4 (4 bytes) */
	stream_write_UINT32(s, 0); /* pad5 (4 bytes) */
	stream_write_UINT32(s, 0); /* pad6 (4 bytes) */

	size = bpp * 256;
	stream_write_UINT16(s, 200); /* Cache0Entries (2 bytes) */
	stream_write_UINT16(s, size); /* Cache0MaximumCellSize (2 bytes) */

	size = bpp * 1024;
	stream_write_UINT16(s, 600); /* Cache1Entries (2 bytes) */
	stream_write_UINT16(s, size); /* Cache1MaximumCellSize (2 bytes) */

	size = bpp * 4096;
	stream_write_UINT16(s, 1000); /* Cache2Entries (2 bytes) */
	stream_write_UINT16(s, size); /* Cache2MaximumCellSize (2 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_BITMAP_CACHE);
}

/**
 * Read control capability set.\n
 * @msdn{cc240568}
 * @param s stream
 * @param settings settings
 */

void rdp_read_control_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	stream_seek_UINT16(s); /* controlFlags (2 bytes) */
	stream_seek_UINT16(s); /* remoteDetachFlag (2 bytes) */
	stream_seek_UINT16(s); /* controlInterest (2 bytes) */
	stream_seek_UINT16(s); /* detachInterest (2 bytes) */
}

/**
 * Write control capability set.\n
 * @msdn{cc240568}
 * @param s stream
 * @param settings settings
 */

void rdp_write_control_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;

	header = rdp_capability_set_start(s);

	stream_write_UINT16(s, 0); /* controlFlags (2 bytes) */
	stream_write_UINT16(s, 0); /* remoteDetachFlag (2 bytes) */
	stream_write_UINT16(s, 2); /* controlInterest (2 bytes) */
	stream_write_UINT16(s, 2); /* detachInterest (2 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_CONTROL);
}

/**
 * Read window activation capability set.\n
 * @msdn{cc240569}
 * @param s stream
 * @param settings settings
 */

void rdp_read_window_activation_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	stream_seek_UINT16(s); /* helpKeyFlag (2 bytes) */
	stream_seek_UINT16(s); /* helpKeyIndexFlag (2 bytes) */
	stream_seek_UINT16(s); /* helpExtendedKeyFlag (2 bytes) */
	stream_seek_UINT16(s); /* windowManagerKeyFlag (2 bytes) */
}

/**
 * Write window activation capability set.\n
 * @msdn{cc240569}
 * @param s stream
 * @param settings settings
 */

void rdp_write_window_activation_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;

	header = rdp_capability_set_start(s);

	stream_write_UINT16(s, 0); /* helpKeyFlag (2 bytes) */
	stream_write_UINT16(s, 0); /* helpKeyIndexFlag (2 bytes) */
	stream_write_UINT16(s, 0); /* helpExtendedKeyFlag (2 bytes) */
	stream_write_UINT16(s, 0); /* windowManagerKeyFlag (2 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_ACTIVATION);
}

/**
 * Read pointer capability set.\n
 * @msdn{cc240562}
 * @param s stream
 * @param settings settings
 */

void rdp_read_pointer_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	UINT16 colorPointerFlag;
	UINT16 colorPointerCacheSize;
	UINT16 pointerCacheSize;

	stream_read_UINT16(s, colorPointerFlag); /* colorPointerFlag (2 bytes) */
	stream_read_UINT16(s, colorPointerCacheSize); /* colorPointerCacheSize (2 bytes) */
	stream_read_UINT16(s, pointerCacheSize); /* pointerCacheSize (2 bytes) */

	if (colorPointerFlag == FALSE)
		settings->ColorPointerFlag = FALSE;

	if (settings->ServerMode)
	{
		settings->PointerCacheSize = pointerCacheSize;
	}
}

/**
 * Write pointer capability set.\n
 * @msdn{cc240562}
 * @param s stream
 * @param settings settings
 */

void rdp_write_pointer_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT16 colorPointerFlag;

	header = rdp_capability_set_start(s);

	colorPointerFlag = (settings->ColorPointerFlag) ? 1 : 0;

	stream_write_UINT16(s, colorPointerFlag); /* colorPointerFlag (2 bytes) */
	stream_write_UINT16(s, settings->PointerCacheSize); /* colorPointerCacheSize (2 bytes) */

	if (settings->LargePointerFlag)
	{
		stream_write_UINT16(s, settings->PointerCacheSize); /* pointerCacheSize (2 bytes) */
	}

	rdp_capability_set_finish(s, header, CAPSET_TYPE_POINTER);
}

/**
 * Read share capability set.\n
 * @msdn{cc240570}
 * @param s stream
 * @param settings settings
 */

void rdp_read_share_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	stream_seek_UINT16(s); /* nodeId (2 bytes) */
	stream_seek_UINT16(s); /* pad2Octets (2 bytes) */
}

/**
 * Write share capability set.\n
 * @msdn{cc240570}
 * @param s stream
 * @param settings settings
 */

void rdp_write_share_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT16 nodeId;

	header = rdp_capability_set_start(s);

	nodeId = (settings->ServerMode) ? 0x03EA : 0;

	stream_write_UINT16(s, nodeId); /* nodeId (2 bytes) */
	stream_write_UINT16(s, 0); /* pad2Octets (2 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_SHARE);
}

/**
 * Read color cache capability set.\n
 * @msdn{cc241564}
 * @param s stream
 * @param settings settings
 */

void rdp_read_color_cache_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	stream_seek_UINT16(s); /* colorTableCacheSize (2 bytes) */
	stream_seek_UINT16(s); /* pad2Octets (2 bytes) */
}

/**
 * Write color cache capability set.\n
 * @msdn{cc241564}
 * @param s stream
 * @param settings settings
 */

void rdp_write_color_cache_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;

	header = rdp_capability_set_start(s);

	stream_write_UINT16(s, 6); /* colorTableCacheSize (2 bytes) */
	stream_write_UINT16(s, 0); /* pad2Octets (2 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_COLOR_CACHE);
}

/**
 * Read sound capability set.\n
 * @msdn{cc240552}
 * @param s stream
 * @param settings settings
 */

void rdp_read_sound_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	UINT16 soundFlags;

	stream_read_UINT16(s, soundFlags); /* soundFlags (2 bytes) */
	stream_seek_UINT16(s); /* pad2OctetsA (2 bytes) */

	settings->SoundBeepsEnabled = (soundFlags & SOUND_BEEPS_FLAG) ? TRUE : FALSE;
}

/**
 * Write sound capability set.\n
 * @msdn{cc240552}
 * @param s stream
 * @param settings settings
 */

void rdp_write_sound_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT16 soundFlags;

	header = rdp_capability_set_start(s);

	soundFlags = (settings->SoundBeepsEnabled) ? SOUND_BEEPS_FLAG : 0;

	stream_write_UINT16(s, soundFlags); /* soundFlags (2 bytes) */
	stream_write_UINT16(s, 0); /* pad2OctetsA (2 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_SOUND);
}

/**
 * Read input capability set.\n
 * @msdn{cc240563}
 * @param s stream
 * @param settings settings
 */

void rdp_read_input_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	UINT16 inputFlags;

	stream_read_UINT16(s, inputFlags); /* inputFlags (2 bytes) */
	stream_seek_UINT16(s); /* pad2OctetsA (2 bytes) */

	if (settings->ServerMode)
	{
		stream_read_UINT32(s, settings->KeyboardLayout); /* keyboardLayout (4 bytes) */
		stream_read_UINT32(s, settings->KeyboardType); /* keyboardType (4 bytes) */
		stream_read_UINT32(s, settings->KeyboardSubType); /* keyboardSubType (4 bytes) */
		stream_read_UINT32(s, settings->KeyboardFunctionKey); /* keyboardFunctionKeys (4 bytes) */
	}
	else
	{
		stream_seek_UINT32(s); /* keyboardLayout (4 bytes) */
		stream_seek_UINT32(s); /* keyboardType (4 bytes) */
		stream_seek_UINT32(s); /* keyboardSubType (4 bytes) */
		stream_seek_UINT32(s); /* keyboardFunctionKeys (4 bytes) */
	}

	stream_seek(s, 64); /* imeFileName (64 bytes) */

	if (settings->ServerMode != TRUE)
	{
		if (inputFlags & INPUT_FLAG_FASTPATH_INPUT)
		{
			/* advertised by RDP 5.0 and 5.1 servers */
		}
		else if (inputFlags & INPUT_FLAG_FASTPATH_INPUT2)
		{
			/* avertised by RDP 5.2, 6.0, 6.1 and 7.0 servers */
		}
		else
		{
			/* server does not support fastpath input */
			settings->FastPathInput = FALSE;
		}
	}
}

/**
 * Write input capability set.\n
 * @msdn{cc240563}
 * @param s stream
 * @param settings settings
 */

void rdp_write_input_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT16 inputFlags;

	header = rdp_capability_set_start(s);

	inputFlags = INPUT_FLAG_SCANCODES | INPUT_FLAG_MOUSEX | INPUT_FLAG_UNICODE;

	if (settings->FastPathInput)
	{
		inputFlags |= INPUT_FLAG_FASTPATH_INPUT;
		inputFlags |= INPUT_FLAG_FASTPATH_INPUT2;
	}

	stream_write_UINT16(s, inputFlags); /* inputFlags (2 bytes) */
	stream_write_UINT16(s, 0); /* pad2OctetsA (2 bytes) */
	stream_write_UINT32(s, settings->KeyboardLayout); /* keyboardLayout (4 bytes) */
	stream_write_UINT32(s, settings->KeyboardType); /* keyboardType (4 bytes) */
	stream_write_UINT32(s, settings->KeyboardSubType); /* keyboardSubType (4 bytes) */
	stream_write_UINT32(s, settings->KeyboardFunctionKey); /* keyboardFunctionKeys (4 bytes) */
	stream_write_zero(s, 64); /* imeFileName (64 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_INPUT);
}

/**
 * Read font capability set.\n
 * @msdn{cc240571}
 * @param s stream
 * @param settings settings
 */

void rdp_read_font_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	if (length > 4)
		stream_seek_UINT16(s); /* fontSupportFlags (2 bytes) */

	if (length > 6)
		stream_seek_UINT16(s); /* pad2Octets (2 bytes) */
}

/**
 * Write font capability set.\n
 * @msdn{cc240571}
 * @param s stream
 * @param settings settings
 */

void rdp_write_font_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;

	header = rdp_capability_set_start(s);

	stream_write_UINT16(s, FONTSUPPORT_FONTLIST); /* fontSupportFlags (2 bytes) */
	stream_write_UINT16(s, 0); /* pad2Octets (2 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_FONT);
}

/**
 * Read brush capability set.\n
 * @msdn{cc240564}
 * @param s stream
 * @param settings settings
 */

void rdp_read_brush_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	stream_seek_UINT32(s); /* brushSupportLevel (4 bytes) */
}

/**
 * Write brush capability set.\n
 * @msdn{cc240564}
 * @param s stream
 * @param settings settings
 */

void rdp_write_brush_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;

	header = rdp_capability_set_start(s);

	stream_write_UINT32(s, BRUSH_COLOR_FULL); /* brushSupportLevel (4 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_BRUSH);
}

/**
 * Read cache definition (glyph).\n
 * @msdn{cc240566}
 * @param s stream
 */
void rdp_read_cache_definition(STREAM* s, GLYPH_CACHE_DEFINITION* cache_definition)
{
	stream_read_UINT16(s, cache_definition->cacheEntries); /* cacheEntries (2 bytes) */
	stream_read_UINT16(s, cache_definition->cacheMaximumCellSize); /* cacheMaximumCellSize (2 bytes) */
}

/**
 * Write cache definition (glyph).\n
 * @msdn{cc240566}
 * @param s stream
 */
void rdp_write_cache_definition(STREAM* s, GLYPH_CACHE_DEFINITION* cache_definition)
{
	stream_write_UINT16(s, cache_definition->cacheEntries); /* cacheEntries (2 bytes) */
	stream_write_UINT16(s, cache_definition->cacheMaximumCellSize); /* cacheMaximumCellSize (2 bytes) */
}

/**
 * Read glyph cache capability set.\n
 * @msdn{cc240565}
 * @param s stream
 * @param settings settings
 */

void rdp_read_glyph_cache_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	UINT16 glyphSupportLevel;

	stream_seek(s, 40); /* glyphCache (40 bytes) */
	stream_seek_UINT32(s); /* fragCache (4 bytes) */
	stream_read_UINT16(s, glyphSupportLevel); /* glyphSupportLevel (2 bytes) */
	stream_seek_UINT16(s); /* pad2Octets (2 bytes) */

	settings->GlyphSupportLevel = glyphSupportLevel;
}

/**
 * Write glyph cache capability set.\n
 * @msdn{cc240565}
 * @param s stream
 * @param settings settings
 */

void rdp_write_glyph_cache_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;

	header = rdp_capability_set_start(s);

	/* glyphCache (40 bytes) */
	rdp_write_cache_definition(s, &(settings->GlyphCache[0])); /* glyphCache0 (4 bytes) */
	rdp_write_cache_definition(s, &(settings->GlyphCache[1])); /* glyphCache1 (4 bytes) */
	rdp_write_cache_definition(s, &(settings->GlyphCache[2])); /* glyphCache2 (4 bytes) */
	rdp_write_cache_definition(s, &(settings->GlyphCache[3])); /* glyphCache3 (4 bytes) */
	rdp_write_cache_definition(s, &(settings->GlyphCache[4])); /* glyphCache4 (4 bytes) */
	rdp_write_cache_definition(s, &(settings->GlyphCache[5])); /* glyphCache5 (4 bytes) */
	rdp_write_cache_definition(s, &(settings->GlyphCache[6])); /* glyphCache6 (4 bytes) */
	rdp_write_cache_definition(s, &(settings->GlyphCache[7])); /* glyphCache7 (4 bytes) */
	rdp_write_cache_definition(s, &(settings->GlyphCache[8])); /* glyphCache8 (4 bytes) */
	rdp_write_cache_definition(s, &(settings->GlyphCache[9])); /* glyphCache9 (4 bytes) */

	rdp_write_cache_definition(s, settings->FragCache);  /* fragCache (4 bytes) */

	stream_write_UINT16(s, settings->GlyphSupportLevel); /* glyphSupportLevel (2 bytes) */

	stream_write_UINT16(s, 0); /* pad2Octets (2 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_GLYPH_CACHE);
}

/**
 * Read offscreen bitmap cache capability set.\n
 * @msdn{cc240550}
 * @param s stream
 * @param settings settings
 */

void rdp_read_offscreen_bitmap_cache_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	UINT32 offscreenSupportLevel;

	stream_read_UINT32(s, offscreenSupportLevel); /* offscreenSupportLevel (4 bytes) */
	stream_read_UINT16(s, settings->OffscreenCacheSize); /* offscreenCacheSize (2 bytes) */
	stream_read_UINT16(s, settings->OffscreenCacheEntries); /* offscreenCacheEntries (2 bytes) */

	if (offscreenSupportLevel & TRUE)
		settings->OffscreenSupportLevel = TRUE;
}

/**
 * Write offscreen bitmap cache capability set.\n
 * @msdn{cc240550}
 * @param s stream
 * @param settings settings
 */

void rdp_write_offscreen_bitmap_cache_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT32 offscreenSupportLevel = FALSE;

	header = rdp_capability_set_start(s);

	if (settings->OffscreenSupportLevel)
		offscreenSupportLevel = TRUE;

	stream_write_UINT32(s, offscreenSupportLevel); /* offscreenSupportLevel (4 bytes) */
	stream_write_UINT16(s, settings->OffscreenCacheSize); /* offscreenCacheSize (2 bytes) */
	stream_write_UINT16(s, settings->OffscreenCacheEntries); /* offscreenCacheEntries (2 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_OFFSCREEN_CACHE);
}

/**
 * Read bitmap cache host support capability set.\n
 * @msdn{cc240557}
 * @param s stream
 * @param settings settings
 */

void rdp_read_bitmap_cache_host_support_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	BYTE cacheVersion;

	stream_read_BYTE(s, cacheVersion); /* cacheVersion (1 byte) */
	stream_seek_BYTE(s); /* pad1 (1 byte) */
	stream_seek_UINT16(s); /* pad2 (2 bytes) */

	if (cacheVersion & BITMAP_CACHE_V2)
		settings->BitmapCachePersistEnabled = TRUE;
}

/**
 * Write bitmap cache host support capability set.\n
 * @msdn{cc240557}
 * @param s stream
 * @param settings settings
 */

void rdp_write_bitmap_cache_host_support_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;

	header = rdp_capability_set_start(s);

	stream_write_BYTE(s, BITMAP_CACHE_V2); /* cacheVersion (1 byte) */
	stream_write_BYTE(s, 0); /* pad1 (1 byte) */
	stream_write_UINT16(s, 0); /* pad2 (2 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_BITMAP_CACHE_HOST_SUPPORT);
}

void rdp_write_bitmap_cache_cell_info(STREAM* s, BITMAP_CACHE_V2_CELL_INFO* cellInfo)
{
	UINT32 info;

	/**
	 * numEntries is in the first 31 bits, while the last bit (k)
	 * is used to indicate a persistent bitmap cache.
	 */

	info = (cellInfo->numEntries | (cellInfo->persistent << 31));
	stream_write_UINT32(s, info);
}

/**
 * Read bitmap cache v2 capability set.\n
 * @msdn{cc240560}
 * @param s stream
 * @param settings settings
 */

void rdp_read_bitmap_cache_v2_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	stream_seek_UINT16(s); /* cacheFlags (2 bytes) */
	stream_seek_BYTE(s); /* pad2 (1 byte) */
	stream_seek_BYTE(s); /* numCellCaches (1 byte) */
	stream_seek(s, 4); /* bitmapCache0CellInfo (4 bytes) */
	stream_seek(s, 4); /* bitmapCache1CellInfo (4 bytes) */
	stream_seek(s, 4); /* bitmapCache2CellInfo (4 bytes) */
	stream_seek(s, 4); /* bitmapCache3CellInfo (4 bytes) */
	stream_seek(s, 4); /* bitmapCache4CellInfo (4 bytes) */
	stream_seek(s, 12); /* pad3 (12 bytes) */
}

/**
 * Write bitmap cache v2 capability set.\n
 * @msdn{cc240560}
 * @param s stream
 * @param settings settings
 */

void rdp_write_bitmap_cache_v2_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT16 cacheFlags;

	header = rdp_capability_set_start(s);

	cacheFlags = ALLOW_CACHE_WAITING_LIST_FLAG;

	if (settings->BitmapCachePersistEnabled)
		cacheFlags |= PERSISTENT_KEYS_EXPECTED_FLAG;

	stream_write_UINT16(s, cacheFlags); /* cacheFlags (2 bytes) */
	stream_write_BYTE(s, 0); /* pad2 (1 byte) */
	stream_write_BYTE(s, settings->BitmapCacheV2NumCells); /* numCellCaches (1 byte) */
	rdp_write_bitmap_cache_cell_info(s, &settings->BitmapCacheV2CellInfo[0]); /* bitmapCache0CellInfo (4 bytes) */
	rdp_write_bitmap_cache_cell_info(s, &settings->BitmapCacheV2CellInfo[1]); /* bitmapCache1CellInfo (4 bytes) */
	rdp_write_bitmap_cache_cell_info(s, &settings->BitmapCacheV2CellInfo[2]); /* bitmapCache2CellInfo (4 bytes) */
	rdp_write_bitmap_cache_cell_info(s, &settings->BitmapCacheV2CellInfo[3]); /* bitmapCache3CellInfo (4 bytes) */
	rdp_write_bitmap_cache_cell_info(s, &settings->BitmapCacheV2CellInfo[4]); /* bitmapCache4CellInfo (4 bytes) */
	stream_write_zero(s, 12); /* pad3 (12 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_BITMAP_CACHE_V2);
}

/**
 * Read virtual channel capability set.\n
 * @msdn{cc240551}
 * @param s stream
 * @param settings settings
 */

void rdp_read_virtual_channel_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	UINT32 flags;
	UINT32 VCChunkSize;

	stream_read_UINT32(s, flags); /* flags (4 bytes) */

	if (length > 8)
		stream_read_UINT32(s, VCChunkSize); /* VCChunkSize (4 bytes) */
	else
		VCChunkSize = 1600;

	if (settings->ServerMode != TRUE)
		settings->VirtualChannelChunkSize = VCChunkSize;
}

/**
 * Write virtual channel capability set.\n
 * @msdn{cc240551}
 * @param s stream
 * @param settings settings
 */

void rdp_write_virtual_channel_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT32 flags;

	header = rdp_capability_set_start(s);

	flags = VCCAPS_NO_COMPR;

	stream_write_UINT32(s, flags); /* flags (4 bytes) */
	stream_write_UINT32(s, settings->VirtualChannelChunkSize); /* VCChunkSize (4 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_VIRTUAL_CHANNEL);
}

/**
 * Read drawn nine grid cache capability set.\n
 * @msdn{cc241565}
 * @param s stream
 * @param settings settings
 */

void rdp_read_draw_nine_grid_cache_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	UINT32 drawNineGridSupportLevel;

	stream_read_UINT32(s, drawNineGridSupportLevel); /* drawNineGridSupportLevel (4 bytes) */
	stream_read_UINT16(s, settings->DrawNineGridCacheSize); /* drawNineGridCacheSize (2 bytes) */
	stream_read_UINT16(s, settings->DrawNineGridCacheEntries); /* drawNineGridCacheEntries (2 bytes) */

	if ((drawNineGridSupportLevel & DRAW_NINEGRID_SUPPORTED) ||
			(drawNineGridSupportLevel & DRAW_NINEGRID_SUPPORTED_V2))
		settings->DrawNineGridEnabled = TRUE;
}

/**
 * Write drawn nine grid cache capability set.\n
 * @msdn{cc241565}
 * @param s stream
 * @param settings settings
 */

void rdp_write_draw_nine_grid_cache_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT32 drawNineGridSupportLevel;

	header = rdp_capability_set_start(s);

	drawNineGridSupportLevel = (settings->DrawNineGridEnabled) ? DRAW_NINEGRID_SUPPORTED_V2 : DRAW_NINEGRID_NO_SUPPORT;

	stream_write_UINT32(s, drawNineGridSupportLevel); /* drawNineGridSupportLevel (4 bytes) */
	stream_write_UINT16(s, settings->DrawNineGridCacheSize); /* drawNineGridCacheSize (2 bytes) */
	stream_write_UINT16(s, settings->DrawNineGridCacheEntries); /* drawNineGridCacheEntries (2 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_DRAW_NINE_GRID_CACHE);
}

void rdp_write_gdiplus_cache_entries(STREAM* s, UINT16 gce, UINT16 bce, UINT16 pce, UINT16 ice, UINT16 ace)
{
	stream_write_UINT16(s, gce); /* gdipGraphicsCacheEntries (2 bytes) */
	stream_write_UINT16(s, bce); /* gdipBrushCacheEntries (2 bytes) */
	stream_write_UINT16(s, pce); /* gdipPenCacheEntries (2 bytes) */
	stream_write_UINT16(s, ice); /* gdipImageCacheEntries (2 bytes) */
	stream_write_UINT16(s, ace); /* gdipImageAttributesCacheEntries (2 bytes) */
}

void rdp_write_gdiplus_cache_chunk_size(STREAM* s, UINT16 gccs, UINT16 obccs, UINT16 opccs, UINT16 oiaccs)
{
	stream_write_UINT16(s, gccs); /* gdipGraphicsCacheChunkSize (2 bytes) */
	stream_write_UINT16(s, obccs); /* gdipObjectBrushCacheChunkSize (2 bytes) */
	stream_write_UINT16(s, opccs); /* gdipObjectPenCacheChunkSize (2 bytes) */
	stream_write_UINT16(s, oiaccs); /* gdipObjectImageAttributesCacheChunkSize (2 bytes) */
}

void rdp_write_gdiplus_image_cache_properties(STREAM* s, UINT16 oiccs, UINT16 oicts, UINT16 oicms)
{
	stream_write_UINT16(s, oiccs); /* gdipObjectImageCacheChunkSize (2 bytes) */
	stream_write_UINT16(s, oicts); /* gdipObjectImageCacheTotalSize (2 bytes) */
	stream_write_UINT16(s, oicms); /* gdipObjectImageCacheMaxSize (2 bytes) */
}

/**
 * Read GDI+ cache capability set.\n
 * @msdn{cc241566}
 * @param s stream
 * @param settings settings
 */

void rdp_read_draw_gdiplus_cache_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	UINT32 drawGDIPlusSupportLevel;
	UINT32 drawGdiplusCacheLevel;

	stream_read_UINT32(s, drawGDIPlusSupportLevel); /* drawGDIPlusSupportLevel (4 bytes) */
	stream_seek_UINT32(s); /* GdipVersion (4 bytes) */
	stream_read_UINT32(s, drawGdiplusCacheLevel); /* drawGdiplusCacheLevel (4 bytes) */
	stream_seek(s, 10); /* GdipCacheEntries (10 bytes) */
	stream_seek(s, 8); /* GdipCacheChunkSize (8 bytes) */
	stream_seek(s, 6); /* GdipImageCacheProperties (6 bytes) */

	if (drawGDIPlusSupportLevel & DRAW_GDIPLUS_SUPPORTED)
		settings->DrawGdiPlusEnabled = TRUE;

	if (drawGdiplusCacheLevel & DRAW_GDIPLUS_CACHE_LEVEL_ONE)
		settings->DrawGdiPlusCacheEnabled = TRUE;
}

/**
 * Write GDI+ cache capability set.\n
 * @msdn{cc241566}
 * @param s stream
 * @param settings settings
 */

void rdp_write_draw_gdiplus_cache_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT32 drawGDIPlusSupportLevel;
	UINT32 drawGdiplusCacheLevel;

	header = rdp_capability_set_start(s);

	drawGDIPlusSupportLevel = (settings->DrawGdiPlusEnabled) ? DRAW_GDIPLUS_SUPPORTED : DRAW_GDIPLUS_DEFAULT;
	drawGdiplusCacheLevel = (settings->DrawGdiPlusEnabled) ? DRAW_GDIPLUS_CACHE_LEVEL_ONE : DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT;

	stream_write_UINT32(s, drawGDIPlusSupportLevel); /* drawGDIPlusSupportLevel (4 bytes) */
	stream_write_UINT32(s, 0); /* GdipVersion (4 bytes) */
	stream_write_UINT32(s, drawGdiplusCacheLevel); /* drawGdiplusCacheLevel (4 bytes) */
	rdp_write_gdiplus_cache_entries(s, 10, 5, 5, 10, 2); /* GdipCacheEntries (10 bytes) */
	rdp_write_gdiplus_cache_chunk_size(s, 512, 2048, 1024, 64); /* GdipCacheChunkSize (8 bytes) */
	rdp_write_gdiplus_image_cache_properties(s, 4096, 256, 128); /* GdipImageCacheProperties (6 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_DRAW_GDI_PLUS);
}

/**
 * Read remote programs capability set.\n
 * @msdn{cc242518}
 * @param s stream
 * @param settings settings
 */

void rdp_read_remote_programs_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	UINT32 railSupportLevel;

	stream_read_UINT32(s, railSupportLevel); /* railSupportLevel (4 bytes) */

	if ((railSupportLevel & RAIL_LEVEL_SUPPORTED) == 0)
	{
		if (settings->RemoteApplicationMode == TRUE)
		{
			/* RemoteApp Failure! */
			settings->RemoteApplicationMode = FALSE;
		}
	}
}

/**
 * Write remote programs capability set.\n
 * @msdn{cc242518}
 * @param s stream
 * @param settings settings
 */

void rdp_write_remote_programs_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT32 railSupportLevel;

	header = rdp_capability_set_start(s);

	railSupportLevel = RAIL_LEVEL_SUPPORTED;

	if (settings->RemoteAppLanguageBarSupported)
		railSupportLevel |= RAIL_LEVEL_DOCKED_LANGBAR_SUPPORTED;

	stream_write_UINT32(s, railSupportLevel); /* railSupportLevel (4 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_RAIL);
}

/**
 * Read window list capability set.\n
 * @msdn{cc242564}
 * @param s stream
 * @param settings settings
 */

void rdp_read_window_list_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	stream_seek_UINT32(s); /* wndSupportLevel (4 bytes) */
	stream_seek_BYTE(s); /* numIconCaches (1 byte) */
	stream_seek_UINT16(s); /* numIconCacheEntries (2 bytes) */
}

/**
 * Write window list capability set.\n
 * @msdn{cc242564}
 * @param s stream
 * @param settings settings
 */

void rdp_write_window_list_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT32 wndSupportLevel;

	header = rdp_capability_set_start(s);

	wndSupportLevel = WINDOW_LEVEL_SUPPORTED_EX;

	stream_write_UINT32(s, wndSupportLevel); /* wndSupportLevel (4 bytes) */
	stream_write_BYTE(s, settings->RemoteAppNumIconCaches); /* numIconCaches (1 byte) */
	stream_write_UINT16(s, settings->RemoteAppNumIconCacheEntries); /* numIconCacheEntries (2 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_WINDOW);
}

/**
 * Read desktop composition capability set.\n
 * @msdn{cc240855}
 * @param s stream
 * @param settings settings
 */

void rdp_read_desktop_composition_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	stream_seek_UINT16(s); /* compDeskSupportLevel (2 bytes) */
}

/**
 * Write desktop composition capability set.\n
 * @msdn{cc240855}
 * @param s stream
 * @param settings settings
 */

void rdp_write_desktop_composition_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT16 compDeskSupportLevel;

	header = rdp_capability_set_start(s);

	compDeskSupportLevel = (settings->AllowDesktopComposition) ? COMPDESK_SUPPORTED : COMPDESK_NOT_SUPPORTED;

	stream_write_UINT16(s, compDeskSupportLevel); /* compDeskSupportLevel (2 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_COMP_DESK);
}

/**
 * Read multifragment update capability set.\n
 * @msdn{cc240649}
 * @param s stream
 * @param settings settings
 */

void rdp_read_multifragment_update_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	stream_read_UINT32(s, settings->MultifragMaxRequestSize); /* MaxRequestSize (4 bytes) */
}

/**
 * Write multifragment update capability set.\n
 * @msdn{cc240649}
 * @param s stream
 * @param settings settings
 */

void rdp_write_multifragment_update_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;

	header = rdp_capability_set_start(s);

	stream_write_UINT32(s, settings->MultifragMaxRequestSize); /* MaxRequestSize (4 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_MULTI_FRAGMENT_UPDATE);
}

/**
 * Read large pointer capability set.\n
 * @msdn{cc240650}
 * @param s stream
 * @param settings settings
 */

void rdp_read_large_pointer_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	stream_seek_UINT16(s); /* largePointerSupportFlags (2 bytes) */
}

/**
 * Write large pointer capability set.\n
 * @msdn{cc240650}
 * @param s stream
 * @param settings settings
 */

void rdp_write_large_pointer_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT16 largePointerSupportFlags;

	header = rdp_capability_set_start(s);

	largePointerSupportFlags = (settings->LargePointerFlag) ? LARGE_POINTER_FLAG_96x96 : 0;

	stream_write_UINT16(s, largePointerSupportFlags); /* largePointerSupportFlags (2 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_LARGE_POINTER);
}

/**
 * Read surface commands capability set.\n
 * @msdn{dd871563}
 * @param s stream
 * @param settings settings
 */

void rdp_read_surface_commands_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	stream_seek_UINT32(s); /* cmdFlags (4 bytes) */
	stream_seek_UINT32(s); /* reserved (4 bytes) */

	settings->SurfaceCommandsEnabled = TRUE;
}

/**
 * Write surface commands capability set.\n
 * @msdn{dd871563}
 * @param s stream
 * @param settings settings
 */

void rdp_write_surface_commands_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT32 cmdFlags;

	header = rdp_capability_set_start(s);

	cmdFlags = SURFCMDS_FRAME_MARKER |
			SURFCMDS_SET_SURFACE_BITS |
			SURFCMDS_STREAM_SURFACE_BITS;

	stream_write_UINT32(s, cmdFlags); /* cmdFlags (4 bytes) */
	stream_write_UINT32(s, 0); /* reserved (4 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_SURFACE_COMMANDS);
}

/**
 * Read bitmap codecs capability set.\n
 * @msdn{dd891377}
 * @param s stream
 * @param settings settings
 */

void rdp_read_bitmap_codecs_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	BYTE bitmapCodecCount;
	UINT16 codecPropertiesLength;

	stream_read_BYTE(s, bitmapCodecCount); /* bitmapCodecCount (1 byte) */

	if (settings->ServerMode)
	{
		settings->RemoteFxCodec = FALSE;
		settings->NSCodec = FALSE;
		settings->JpegCodec = FALSE;
	}

	while (bitmapCodecCount > 0)
	{
		if (settings->ServerMode && strncmp((char*)stream_get_tail(s), CODEC_GUID_REMOTEFX, 16) == 0)
		{
			stream_seek(s, 16); /* codecGUID (16 bytes) */
			stream_read_BYTE(s, settings->RemoteFxCodecId);
			settings->RemoteFxCodec = TRUE;
		}
		else if (settings->ServerMode && strncmp((char*)stream_get_tail(s), CODEC_GUID_NSCODEC, 16) == 0)
		{
			stream_seek(s, 16); /*codec GUID (16 bytes) */
			stream_read_BYTE(s, settings->NSCodecId);
			settings->NSCodec = TRUE;
		}
		else
		{
			stream_seek(s, 16); /* codecGUID (16 bytes) */
			stream_seek_BYTE(s); /* codecID (1 byte) */
		}

		stream_read_UINT16(s, codecPropertiesLength); /* codecPropertiesLength (2 bytes) */
		stream_seek(s, codecPropertiesLength); /* codecProperties */

		bitmapCodecCount--;
	}
}

/**
 * Write RemoteFX Client Capability Container.\n
 * @param s stream
 * @param settings settings
 */
void rdp_write_rfx_client_capability_container(STREAM* s, rdpSettings* settings)
{
	UINT32 captureFlags;
	BYTE codecMode;

	captureFlags = settings->RemoteFxOnly ? CARDP_CAPS_CAPTURE_NON_CAC : 0;
	codecMode = settings->RemoteFxCodecMode;

	stream_write_UINT16(s, 49); /* codecPropertiesLength */

	/* TS_RFX_CLNT_CAPS_CONTAINER */
	stream_write_UINT32(s, 49); /* length */
	stream_write_UINT32(s, captureFlags); /* captureFlags */
	stream_write_UINT32(s, 37); /* capsLength */

	/* TS_RFX_CAPS */
	stream_write_UINT16(s, CBY_CAPS); /* blockType */
	stream_write_UINT32(s, 8); /* blockLen */
	stream_write_UINT16(s, 1); /* numCapsets */

	/* TS_RFX_CAPSET */
	stream_write_UINT16(s, CBY_CAPSET); /* blockType */
	stream_write_UINT32(s, 29); /* blockLen */
	stream_write_BYTE(s, 0x01); /* codecId (MUST be set to 0x01) */
	stream_write_UINT16(s, CLY_CAPSET); /* capsetType */
	stream_write_UINT16(s, 2); /* numIcaps */
	stream_write_UINT16(s, 8); /* icapLen */

	/* TS_RFX_ICAP (RLGR1) */
	stream_write_UINT16(s, CLW_VERSION_1_0); /* version */
	stream_write_UINT16(s, CT_TILE_64x64); /* tileSize */
	stream_write_BYTE(s, codecMode); /* flags */
	stream_write_BYTE(s, CLW_COL_CONV_ICT); /* colConvBits */
	stream_write_BYTE(s, CLW_XFORM_DWT_53_A); /* transformBits */
	stream_write_BYTE(s, CLW_ENTROPY_RLGR1); /* entropyBits */

	/* TS_RFX_ICAP (RLGR3) */
	stream_write_UINT16(s, CLW_VERSION_1_0); /* version */
	stream_write_UINT16(s, CT_TILE_64x64); /* tileSize */
	stream_write_BYTE(s, codecMode); /* flags */
	stream_write_BYTE(s, CLW_COL_CONV_ICT); /* colConvBits */
	stream_write_BYTE(s, CLW_XFORM_DWT_53_A); /* transformBits */
	stream_write_BYTE(s, CLW_ENTROPY_RLGR3); /* entropyBits */
}

/**
 * Write NSCODEC Client Capability Container.\n
 * @param s stream
 * @param settings settings
 */
void rdp_write_nsc_client_capability_container(STREAM* s, rdpSettings* settings)
{
	stream_write_UINT16(s, 3); /* codecPropertiesLength */

	/* TS_NSCODEC_CAPABILITYSET */
	stream_write_BYTE(s, 1);  /* fAllowDynamicFidelity */
	stream_write_BYTE(s, 1);  /* fAllowSubsampling */
	stream_write_BYTE(s, 3);  /* colorLossLevel */
}

void rdp_write_jpeg_client_capability_container(STREAM* s, rdpSettings* settings)
{
	stream_write_UINT16(s, 1); /* codecPropertiesLength */
	stream_write_BYTE(s, settings->JpegQuality);
}

/**
 * Write RemoteFX Server Capability Container.\n
 * @param s stream
 * @param settings settings
 */
void rdp_write_rfx_server_capability_container(STREAM* s, rdpSettings* settings)
{
	stream_write_UINT16(s, 4); /* codecPropertiesLength */
	stream_write_UINT32(s, 0); /* reserved */
}

void rdp_write_jpeg_server_capability_container(STREAM* s, rdpSettings* settings)
{
	stream_write_UINT16(s, 1); /* codecPropertiesLength */
	stream_write_BYTE(s, 75);
}

/**
 * Write NSCODEC Server Capability Container.\n
 * @param s stream
 * @param settings settings
 */
void rdp_write_nsc_server_capability_container(STREAM* s, rdpSettings* settings)
{
	stream_write_UINT16(s, 4); /* codecPropertiesLength */
	stream_write_UINT32(s, 0); /* reserved */
}

/**
 * Write bitmap codecs capability set.\n
 * @msdn{dd891377}
 * @param s stream
 * @param settings settings
 */

void rdp_write_bitmap_codecs_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	BYTE bitmapCodecCount;

	header = rdp_capability_set_start(s);

	bitmapCodecCount = 0;

	if (settings->RemoteFxCodec)
		bitmapCodecCount++;
	if (settings->NSCodec)
		bitmapCodecCount++;
	if (settings->JpegCodec)
		bitmapCodecCount++;

	stream_write_BYTE(s, bitmapCodecCount);

	if (settings->RemoteFxCodec)
	{
		stream_write(s, CODEC_GUID_REMOTEFX, 16); /* codecGUID */

		if (settings->ServerMode)
		{
			stream_write_BYTE(s, 0); /* codecID is defined by the client */
			rdp_write_rfx_server_capability_container(s, settings);
		}
		else
		{
			stream_write_BYTE(s, CODEC_ID_REMOTEFX); /* codecID */
			rdp_write_rfx_client_capability_container(s, settings);
		}
	}
	if (settings->NSCodec)
	{
		stream_write(s, CODEC_GUID_NSCODEC, 16);
		if (settings->ServerMode)
		{
			stream_write_BYTE(s, 0); /* codecID is defined by the client */
			rdp_write_nsc_server_capability_container(s, settings);
		}
		else
		{
			stream_write_BYTE(s, CODEC_ID_NSCODEC); /* codecID */
			rdp_write_nsc_client_capability_container(s, settings);
		}
	}
	if (settings->JpegCodec)
	{
		stream_write(s, CODEC_GUID_JPEG, 16);
		if (settings->ServerMode)
		{
			stream_write_BYTE(s, 0); /* codecID is defined by the client */
			rdp_write_jpeg_server_capability_container(s, settings);
		}
		else
		{
			stream_write_BYTE(s, CODEC_ID_JPEG); /* codecID */
			rdp_write_jpeg_client_capability_container(s, settings);
		}
	}
	rdp_capability_set_finish(s, header, CAPSET_TYPE_BITMAP_CODECS);
}

/**
 * Read frame acknowledge capability set.\n
 * @param s stream
 * @param settings settings
 */

void rdp_read_frame_acknowledge_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	if (settings->ServerMode)
	{
		stream_read_UINT32(s, settings->FrameAcknowledge); /* (4 bytes) */
	}
	else
	{
		stream_seek_UINT32(s); /* (4 bytes) */
	}
}

void rdp_read_bitmap_cache_v3_codec_id_capability_set(STREAM* s, UINT16 length, rdpSettings* settings)
{
	stream_seek_BYTE(s); /* (1 byte) */
}

void rdp_write_bitmap_cache_v3_codec_id_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;

	header = rdp_capability_set_start(s);
	stream_write_BYTE(s, settings->BitmapCacheV3CodecId);
	rdp_capability_set_finish(s, header, 6);
}


/**
 * Write frame acknowledge capability set.\n
 * @param s stream
 * @param settings settings
 */

void rdp_write_frame_acknowledge_capability_set(STREAM* s, rdpSettings* settings)
{
	BYTE* header;
	UINT32 frame_acknowledge;

	header = rdp_capability_set_start(s);

	frame_acknowledge = settings->FrameAcknowledge;
	stream_write_UINT32(s, frame_acknowledge); /* (4 bytes) */

	rdp_capability_set_finish(s, header, CAPSET_TYPE_FRAME_ACKNOWLEDGE);
}

BOOL rdp_read_capability_sets(STREAM* s, rdpSettings* settings, UINT16 numberCapabilities)
{
	UINT16 type;
	UINT16 length;
	BYTE *bm, *em;

	while (numberCapabilities > 0)
	{
		stream_get_mark(s, bm);

		rdp_read_capability_set_header(s, &length, &type);
		//printf("%s Capability Set (0x%02X), length:%d\n", CAPSET_TYPE_STRINGS[type], type, length);
		settings->ReceivedCapabilities[type] = TRUE;
		em = bm + length;

		if (stream_get_left(s) < length - 4)
		{
			printf("error processing stream\n");
			return FALSE;
		}

		switch (type)
		{
			case CAPSET_TYPE_GENERAL:
				rdp_read_general_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_BITMAP:
				rdp_read_bitmap_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_ORDER:
				rdp_read_order_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_BITMAP_CACHE:
				rdp_read_bitmap_cache_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_CONTROL:
				rdp_read_control_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_ACTIVATION:
				rdp_read_window_activation_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_POINTER:
				rdp_read_pointer_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_SHARE:
				rdp_read_share_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_COLOR_CACHE:
				rdp_read_color_cache_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_SOUND:
				rdp_read_sound_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_INPUT:
				rdp_read_input_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_FONT:
				rdp_read_font_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_BRUSH:
				rdp_read_brush_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_GLYPH_CACHE:
				rdp_read_glyph_cache_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_OFFSCREEN_CACHE:
				rdp_read_offscreen_bitmap_cache_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_BITMAP_CACHE_HOST_SUPPORT:
				rdp_read_bitmap_cache_host_support_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_BITMAP_CACHE_V2:
				rdp_read_bitmap_cache_v2_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_VIRTUAL_CHANNEL:
				rdp_read_virtual_channel_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_DRAW_NINE_GRID_CACHE:
				rdp_read_draw_nine_grid_cache_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_DRAW_GDI_PLUS:
				rdp_read_draw_gdiplus_cache_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_RAIL:
				rdp_read_remote_programs_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_WINDOW:
				rdp_read_window_list_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_COMP_DESK:
				rdp_read_desktop_composition_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_MULTI_FRAGMENT_UPDATE:
				rdp_read_multifragment_update_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_LARGE_POINTER:
				rdp_read_large_pointer_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_SURFACE_COMMANDS:
				rdp_read_surface_commands_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_BITMAP_CODECS:
				rdp_read_bitmap_codecs_capability_set(s, length, settings);
				break;

			case CAPSET_TYPE_FRAME_ACKNOWLEDGE:
				rdp_read_frame_acknowledge_capability_set(s, length, settings);
				break;

			case 6:
				rdp_read_bitmap_cache_v3_codec_id_capability_set(s, length, settings);
				break;

			default:
				printf("unknown capability type %d\n", type);
				break;
		}

		if (s->p != em)
		{
			printf("incorrect offset, type:0x%02X actual:%d expected:%d\n",
				type, (int) (s->p - bm), (int) (em - bm));
		}

		stream_set_mark(s, em);
		numberCapabilities--;
	}

	return TRUE;
}

BOOL rdp_recv_demand_active(rdpRdp* rdp, STREAM* s)
{
	UINT16 length;
	UINT16 channelId;
	UINT16 pduType;
	UINT16 pduLength;
	UINT16 pduSource;
	UINT16 numberCapabilities;
	UINT16 lengthSourceDescriptor;
	UINT16 lengthCombinedCapabilities;
	UINT16 securityFlags;

	if (!rdp_read_header(rdp, s, &length, &channelId))
		return FALSE;

	if (rdp->disconnect)
		return TRUE;

	if (rdp->settings->DisableEncryption)
	{
		rdp_read_security_header(s, &securityFlags);
		if (securityFlags & SEC_ENCRYPT)
		{
			if (!rdp_decrypt(rdp, s, length - 4, securityFlags))
			{
				printf("rdp_decrypt failed\n");
				return FALSE;
			}
		}
	}

	if (channelId != MCS_GLOBAL_CHANNEL_ID)
	{
		printf("expected MCS_GLOBAL_CHANNEL_ID %04x, got %04x\n", MCS_GLOBAL_CHANNEL_ID, channelId);
		return FALSE;
	}

	if (!rdp_read_share_control_header(s, &pduLength, &pduType, &pduSource))
	{
		printf("rdp_read_share_control_header failed\n");
		return FALSE;
	}

	rdp->settings->PduSource = pduSource;

	if (pduType != PDU_TYPE_DEMAND_ACTIVE)
	{
		printf("expected PDU_TYPE_DEMAND_ACTIVE %04x, got %04x\n", PDU_TYPE_DEMAND_ACTIVE, pduType);
		return FALSE;
	}

	stream_read_UINT32(s, rdp->settings->ShareId); /* shareId (4 bytes) */
	stream_read_UINT16(s, lengthSourceDescriptor); /* lengthSourceDescriptor (2 bytes) */
	stream_read_UINT16(s, lengthCombinedCapabilities); /* lengthCombinedCapabilities (2 bytes) */
	stream_seek(s, lengthSourceDescriptor); /* sourceDescriptor */
	stream_read_UINT16(s, numberCapabilities); /* numberCapabilities (2 bytes) */
	stream_seek(s, 2); /* pad2Octets (2 bytes) */

	/* capabilitySets */
	if (!rdp_read_capability_sets(s, rdp->settings, numberCapabilities))
	{
		printf("rdp_read_capability_sets failed\n");
		return FALSE;
	}

	rdp->update->secondary->glyph_v2 = (rdp->settings->GlyphSupportLevel > GLYPH_SUPPORT_FULL) ? TRUE : FALSE;

	return TRUE;
}

void rdp_write_demand_active(STREAM* s, rdpSettings* settings)
{
	BYTE *bm, *em, *lm;
	UINT16 numberCapabilities;
	UINT16 lengthCombinedCapabilities;

	stream_write_UINT32(s, settings->ShareId); /* shareId (4 bytes) */
	stream_write_UINT16(s, 4); /* lengthSourceDescriptor (2 bytes) */

	stream_get_mark(s, lm);
	stream_seek_UINT16(s); /* lengthCombinedCapabilities (2 bytes) */
	stream_write(s, "RDP", 4); /* sourceDescriptor */

	stream_get_mark(s, bm);
	stream_seek_UINT16(s); /* numberCapabilities (2 bytes) */
	stream_write_UINT16(s, 0); /* pad2Octets (2 bytes) */

	numberCapabilities = 14;
	rdp_write_general_capability_set(s, settings);
	rdp_write_bitmap_capability_set(s, settings);
	rdp_write_order_capability_set(s, settings);
	rdp_write_pointer_capability_set(s, settings);
	rdp_write_input_capability_set(s, settings);
	rdp_write_virtual_channel_capability_set(s, settings);
	rdp_write_share_capability_set(s, settings);
	rdp_write_font_capability_set(s, settings);
	rdp_write_multifragment_update_capability_set(s, settings);
	rdp_write_large_pointer_capability_set(s, settings);
	rdp_write_desktop_composition_capability_set(s, settings);
	rdp_write_surface_commands_capability_set(s, settings);
	rdp_write_bitmap_codecs_capability_set(s, settings);
	rdp_write_frame_acknowledge_capability_set(s, settings);

	if (settings->BitmapCachePersistEnabled)
	{
		numberCapabilities++;
		rdp_write_bitmap_cache_host_support_capability_set(s, settings);
	}

	stream_get_mark(s, em);

	stream_set_mark(s, lm); /* go back to lengthCombinedCapabilities */
	lengthCombinedCapabilities = (em - bm);
	stream_write_UINT16(s, lengthCombinedCapabilities); /* lengthCombinedCapabilities (2 bytes) */

	stream_set_mark(s, bm); /* go back to numberCapabilities */
	stream_write_UINT16(s, numberCapabilities); /* numberCapabilities (2 bytes) */

	stream_set_mark(s, em);

	stream_write_UINT32(s, 0); /* sessionId */
}

BOOL rdp_send_demand_active(rdpRdp* rdp)
{
	STREAM* s;

	s = rdp_pdu_init(rdp);

	rdp->settings->ShareId = 0x10000 + rdp->mcs->user_id;

	rdp_write_demand_active(s, rdp->settings);

	rdp_send_pdu(rdp, s, PDU_TYPE_DEMAND_ACTIVE, rdp->mcs->user_id);

	return TRUE;
}

BOOL rdp_recv_confirm_active(rdpRdp* rdp, STREAM* s)
{
	UINT16 length;
	UINT16 channelId;
	UINT16 pduType;
	UINT16 pduLength;
	UINT16 pduSource;
	UINT16 lengthSourceDescriptor;
	UINT16 lengthCombinedCapabilities;
	UINT16 numberCapabilities;
	UINT16 securityFlags;

	if (!rdp_read_header(rdp, s, &length, &channelId))
		return FALSE;

	if (rdp->settings->DisableEncryption)
	{
		rdp_read_security_header(s, &securityFlags);
		if (securityFlags & SEC_ENCRYPT)
		{
			if (!rdp_decrypt(rdp, s, length - 4, securityFlags))
			{
				printf("rdp_decrypt failed\n");
				return FALSE;
			}
		}
	}

	if (channelId != MCS_GLOBAL_CHANNEL_ID)
		return FALSE;

	if (!rdp_read_share_control_header(s, &pduLength, &pduType, &pduSource))
		return FALSE;

	rdp->settings->PduSource = pduSource;

	if (pduType != PDU_TYPE_CONFIRM_ACTIVE)
		return FALSE;

	stream_seek_UINT32(s); /* shareId (4 bytes) */
	stream_seek_UINT16(s); /* originatorId (2 bytes) */
	stream_read_UINT16(s, lengthSourceDescriptor); /* lengthSourceDescriptor (2 bytes) */
	stream_read_UINT16(s, lengthCombinedCapabilities); /* lengthCombinedCapabilities (2 bytes) */
	stream_seek(s, lengthSourceDescriptor); /* sourceDescriptor */
	stream_read_UINT16(s, numberCapabilities); /* numberCapabilities (2 bytes) */
	stream_seek(s, 2); /* pad2Octets (2 bytes) */

	if (!rdp_read_capability_sets(s, rdp->settings, numberCapabilities))
		return FALSE;

	return TRUE;
}

void rdp_write_confirm_active(STREAM* s, rdpSettings* settings)
{
	BYTE *bm, *em, *lm;
	UINT16 numberCapabilities;
	UINT16 lengthSourceDescriptor;
	UINT16 lengthCombinedCapabilities;

	lengthSourceDescriptor = sizeof(SOURCE_DESCRIPTOR);

	stream_write_UINT32(s, settings->ShareId); /* shareId (4 bytes) */
	stream_write_UINT16(s, 0x03EA); /* originatorId (2 bytes) */
	stream_write_UINT16(s, lengthSourceDescriptor);/* lengthSourceDescriptor (2 bytes) */

	stream_get_mark(s, lm);
	stream_seek_UINT16(s); /* lengthCombinedCapabilities (2 bytes) */
	stream_write(s, SOURCE_DESCRIPTOR, lengthSourceDescriptor); /* sourceDescriptor */

	stream_get_mark(s, bm);
	stream_seek_UINT16(s); /* numberCapabilities (2 bytes) */
	stream_write_UINT16(s, 0); /* pad2Octets (2 bytes) */

	/* Capability Sets */
	numberCapabilities = 15;
	rdp_write_general_capability_set(s, settings);
	rdp_write_bitmap_capability_set(s, settings);
	rdp_write_order_capability_set(s, settings);

	if (settings->RdpVersion >= 5)
		rdp_write_bitmap_cache_v2_capability_set(s, settings);
	else
		rdp_write_bitmap_cache_capability_set(s, settings);

	rdp_write_pointer_capability_set(s, settings);
	rdp_write_input_capability_set(s, settings);
	rdp_write_brush_capability_set(s, settings);
	rdp_write_glyph_cache_capability_set(s, settings);
	rdp_write_virtual_channel_capability_set(s, settings);
	rdp_write_sound_capability_set(s, settings);
	rdp_write_share_capability_set(s, settings);
	rdp_write_font_capability_set(s, settings);
	rdp_write_control_capability_set(s, settings);
	rdp_write_color_cache_capability_set(s, settings);
	rdp_write_window_activation_capability_set(s, settings);

	if (settings->OffscreenSupportLevel)
	{
		numberCapabilities++;
		rdp_write_offscreen_bitmap_cache_capability_set(s, settings);
	}

	if (settings->DrawNineGridEnabled)
	{
		numberCapabilities++;
		rdp_write_draw_nine_grid_cache_capability_set(s, settings);
	}

	if (settings->ReceivedCapabilities[CAPSET_TYPE_LARGE_POINTER])
	{
		if (settings->LargePointerFlag)
		{
			numberCapabilities++;
			rdp_write_large_pointer_capability_set(s, settings);
		}
	}

	if (settings->RemoteApplicationMode)
	{
		numberCapabilities += 2;
		rdp_write_remote_programs_capability_set(s, settings);
		rdp_write_window_list_capability_set(s, settings);
	}

	if (settings->ReceivedCapabilities[CAPSET_TYPE_MULTI_FRAGMENT_UPDATE])
	{
		numberCapabilities++;
		rdp_write_multifragment_update_capability_set(s, settings);
	}

	if (settings->ReceivedCapabilities[CAPSET_TYPE_SURFACE_COMMANDS])
	{
		numberCapabilities++;
		rdp_write_surface_commands_capability_set(s, settings);
	}

	if (settings->ReceivedCapabilities[CAPSET_TYPE_BITMAP_CODECS])
	{
		numberCapabilities++;
		rdp_write_bitmap_codecs_capability_set(s, settings);
	}

	if (settings->ReceivedCapabilities[CAPSET_TYPE_FRAME_ACKNOWLEDGE])
	{
		if (settings->FrameAcknowledge > 0)
		{
			numberCapabilities++;
			rdp_write_frame_acknowledge_capability_set(s, settings);
		}
	}

	if (settings->ReceivedCapabilities[6])
	{
		if (settings->BitmapCacheV3CodecId != 0)
		{
			numberCapabilities++;
			rdp_write_bitmap_cache_v3_codec_id_capability_set(s, settings);
		}
	}

	stream_get_mark(s, em);

	stream_set_mark(s, lm); /* go back to lengthCombinedCapabilities */
	lengthCombinedCapabilities = (em - bm);
	stream_write_UINT16(s, lengthCombinedCapabilities); /* lengthCombinedCapabilities (2 bytes) */

	stream_set_mark(s, bm); /* go back to numberCapabilities */
	stream_write_UINT16(s, numberCapabilities); /* numberCapabilities (2 bytes) */

	stream_set_mark(s, em);
}

BOOL rdp_send_confirm_active(rdpRdp* rdp)
{
	STREAM* s;

	s = rdp_pdu_init(rdp);

	rdp_write_confirm_active(s, rdp->settings);

	return rdp_send_pdu(rdp, s, PDU_TYPE_CONFIRM_ACTIVE, rdp->mcs->user_id);
}
