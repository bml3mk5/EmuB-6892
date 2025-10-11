/** @file cocoa_vkeyboard.mm

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2017.01.22 -

 @brief [ virtual keyboard ]
 */

#import "cocoa_vkeyboard.h"
#import "cocoa_basepanel.h"
#import "../../emu.h"
#import "../gui.h"
#import "../../labels.h"
#import "cocoa_key_trans.h"

extern EMU *emu;
extern GUI *gui;

@implementation CocoaVKeyboardView
- (id)initWithSurface:(SDL_Surface *)surface
{
	[super init];

	suf = surface;
	img = nil;

	if (suf != NULL) {
		img = [self allocBuffer];
	}
	if (img != nil) {
		// set size of this view
		[self setFrameSize:[img size]];
		[self copyBuffer];
	} else {
		// cannot convert
		suf = NULL;
	}

	return self;
}
- (void)drawRect:(NSRect)dirtyRect
{
	if (img != nil) {
		NSRect dstRect = [self frame];	// resized size
		NSRect srcRect;
		srcRect.size = [img size];
		srcRect.origin.x = 0;
		srcRect.origin.y = 0;
//		[img drawInRect:dirtyRect];
		[img drawInRect:dstRect fromRect:srcRect operation:NSCompositingOperationCopy fraction:1.0 respectFlipped:NO hints:nil];
	}
	[self setNeedsDisplay:NO];
}
- (NSBitmapImageRep *)allocBuffer
{
	int bps = (int)suf->format->BitsPerPixel / suf->format->BytesPerPixel;
	int spp = 3; // RGB // surface->format->BytesPerPixel;
	int bf = NSAlphaFirstBitmapFormat; // NS32BitLittleEndianBitmapFormat; // NSBitmapFormat;
	return [[NSBitmapImageRep alloc]
		   initWithBitmapDataPlanes:NULL
		   pixelsWide:suf->w
		   pixelsHigh:suf->h
		   bitsPerSample:bps
		   samplesPerPixel:spp
		   hasAlpha:NO isPlanar:NO
		   colorSpaceName:NSDeviceRGBColorSpace
		   bitmapFormat:bf
		   bytesPerRow:suf->pitch
		   bitsPerPixel:suf->format->BitsPerPixel];
}
- (void)copyBuffer
{
	if (suf == NULL || img == nil) return;

	unsigned char *buf = [img bitmapData];
	int size = suf->pitch * suf->h;
	memcpy(buf, suf->pixels, size);

	[self setNeedsDisplay:YES];
}
- (void)copyBufferPart:(NSRect)re
{
	if (suf == NULL || img == nil) return;

	unsigned char *dstbuf = [img bitmapData];
	unsigned char *srcbuf = (unsigned char *)suf->pixels;
	int start = suf->pitch * (int)re.origin.y;
	srcbuf += start;
	dstbuf += start;
	int size = suf->pitch * re.size.height;
	memcpy(dstbuf, srcbuf, size);

	re.origin.y = suf->h - re.size.height - re.origin.y;

//	[self setNeedsDisplayInRect:re];
	// always draw all frame in the view
	[self setNeedsDisplay:YES];
}
#if 0
- (void)viewWillStartLiveResize
{
//	NSRect wre = [[self window] frame];
//	NSRect nre = [self frame];
}
#endif
@end

@implementation CocoaVKeyboard
- (id)initWithSurface:(Vkbd::VKeyboard *)obj surface:(SDL_Surface *)suf
{
	[super init];

	vkeyboard = obj;
	popupMenu = nil;
	if (suf) {
		sufSize.width = suf->w;
		sufSize.height = suf->h;
	} else {
		sufSize.width = 0;
		sufSize.height = 0;
	}

	[self setTitle:@"Virtual Keyboard"];

	// window style
	NSUInteger style = [self styleMask];
	style |= (NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable);
	style &= ~(NSWindowStyleMaskFullScreen); // | NSWindowStyleMaskFullSizeContentView);
	[self setStyleMask:style];

	CocoaVKeyboardView *nview = [[CocoaVKeyboardView alloc] initWithSurface:suf];
	// adjust view size in the window
	NSRect nre = [nview frame];
//	NSRect ore = [[self contentView] frame];
//	NSRect wre = [self frame];
	NSRect wre = [self frameRectForContentRect:nre];
//	wre.size.width += (nre.size.width - ore.size.width);
//	wre.size.height += (nre.size.height - ore.size.height);
	[self setContentView:nview];
	[self setFrame:wre display:YES];
	[self setOpaque:YES];

	// set minimum size
	NSSize min_size = sufSize;
	min_size.width *= 0.25;
	min_size.height *= 0.25;
	[self setContentMinSize:min_size];

	// set maximum size
//	NSSize max_size = sufSize;
//	max_size.width *= 2.0;
//	max_size.height *= 2.0;
//	[self setContentMaxSize:max_size];

	// disable fullscreen
//	[self setMinFullScreenContentSize:min_size];
//	[self setMaxFullScreenContentSize:max_size];

	// always top  
	[self setLevel:NSFloatingWindowLevel];

	// set delegate
	[self setDelegate:[[CocoaVKeyboardDelegate alloc] init]];

//	// hide
//	[self orderOut:nil];

	return self;
}

- (void)close
{
	[super close];
	vkeyboard->PostClose();
}

- (void)setDist
{
	vkeyboard->SetDist();
}

/// left mouse button
- (void)mouseDown:(NSEvent *)theEvent
{
	NSPoint pt = [theEvent locationInWindow];
	NSRect re = [[self contentView] frame];
	vkeyboard->MouseDown(pt.x,  re.size.height - pt.y);
}

- (void)mouseUp:(NSEvent *)theEvent
{
	vkeyboard->MouseUp();
}

/// right mouse button
- (void)rightMouseDown:(NSEvent *)theEvent
{
	[self showPopupMenu];
}

- (void)keyDown:(NSEvent *)theEvent
{
}

- (void)keyUp:(NSEvent *)theEvent
{
}
- (void)changingWindow:(int)num :(NSSize *)size
{
	NSRect vre = [[self contentView] frame];
	double magnify_x = (double)vre.size.width / sufSize.width;
	double magnify_y = (double)vre.size.height / sufSize.height;
#if 0
	if (size) {
//		NSRect wre = [self frame];
//		NSSize margin;
//		margin.width = wre.size.width - vre.size.width;
//		margin.height = wre.size.height - vre.size.height;
		if (magnify_x < 0.25) {
			magnify_x = 0.25;
//			vre.size.width = magnify_x * sufSize.width;
//			size->width = vre.size.width + margin.width;
		}
		if (magnify_y < 0.25) {
			magnify_y = 0.25;
//			vre.size.height = magnify_y * sufSize.height;
//			size->width = vre.size.height + margin.height;
		}
	}
#endif
	vkeyboard->set_magnify(magnify_x, magnify_y);
}
- (void)createPopupMenu
{
	if (popupMenu) return;

	popupMenu = [CocoaMenu create_menu_by_id:CMsg::None_];
	for(int i=0; LABELS::window_size[i].msg_id != CMsg::End; i++) {
		if (LABELS::window_size[i].msg_id != CMsg::Null) {
			[popupMenu add_menu_item_by_id:LABELS::window_size[i].msg_id :self :@selector(selectPopupMenuItem:) :0 :i :0];
		} else {
			[popupMenu addItem:[NSMenuItem separatorItem]];
		}
	}
}
- (void)showPopupMenu
{
	if (!popupMenu) {
		[self createPopupMenu];
	}
	NSEvent *ev = [self currentEvent];
	NSView *vw = [self contentView];
	[NSMenu popUpContextMenu:popupMenu withEvent:ev forView:vw];
}
- (void)selectPopupMenuItem:(id)sender
{
	int num = [sender num];
	vkeyboard->adjust_window_size((double)LABELS::window_size[num].percent / 100.0);
}
@end


@implementation CocoaVKeyboardDelegate
- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize
{
	CocoaVKeyboard *cwindow = (CocoaVKeyboard *)sender;
	[cwindow changingWindow:0:&frameSize];
	return frameSize;
}
- (NSSize)window:(NSWindow *)window willUseFullScreenContentSize:(NSSize)proposedSize;
{
	CocoaVKeyboard *cwindow = (CocoaVKeyboard *)window;
	[cwindow changingWindow:1:&proposedSize];
	return proposedSize;
}
- (void)windowDidExitFullScreen:(NSNotification *)notification
{
	CocoaVKeyboard *cwindow = (CocoaVKeyboard *)[notification object];
	NSSize frameSize = [cwindow frame].size;
	[cwindow changingWindow:2:&frameSize];
}
@end


void vkeyboard_set_owner_window(NSWindow *owner)
{
	// send window object to vkeyboard
	if (owner) {
		NSArray *windows = [NSApp windows];
		for(int i=0; i<[windows count]; i++) {
			NSWindow *window = (NSWindow *)[windows objectAtIndex:i];
			if ([[window className] isEqualToString:@"CocoaVKeyboard"]) {
				[window performSelector:@selector(setOwnerWindow:) withObject:owner];
				[window performSelector:@selector(move)];
				long level = [owner level];
				if (level > 0) {
					[window setLevel:level+1];
				} else {
					[window setLevel:NSFloatingWindowLevel];
				}
			}
		}
	}
}

//
// for Cocoa
//

namespace Vkbd {

VKeyboard::VKeyboard() : OSDBase()
{
	vKbdWin = NULL;
	parentWin = NULL;

//	SDL_QZ_InitOSKeymap();
}

VKeyboard::~VKeyboard()
{
	if (vKbdWin) {
		[vKbdWin release];
		vKbdWin = NULL;
	}
}

bool VKeyboard::Create(const char *res_path)
{
	NSArray *windows = [NSApp windows];
	for(int i=0; i<[windows count]; i++) {
		NSWindow *window = (NSWindow *)[windows objectAtIndex:i];
		if ([[window className] hasPrefix:@"SDL"]) {
			parentWin = window;
			break;
		}
	}

	if (!load_bitmap(res_path)) {
		closed = true;
		return false;
	}

	vKbdWin = [[CocoaVKeyboard alloc] initWithSurface:this surface:pSurface->Get()];
	SetDist();
	closed = false;

	return true;
}

void VKeyboard::Show(bool show)
{
	if (!vKbdWin) return;
	Base::Show(show);

	[vKbdWin orderFront:nil];
	[vKbdWin makeKeyWindow];
}

void VKeyboard::Close()
{
	if (!vKbdWin) return;

	[vKbdWin close];
}

void VKeyboard::PostClose()
{
	vKbdWin = NULL;
	unload_bitmap();
	CloseBase();
}

void VKeyboard::SetDist()
{
	if (!vKbdWin || !parentWin) return;

	NSRect pre = [parentWin frame];
	NSRect re = [vKbdWin frame];
	NSPoint ori;

	ori.x = ((pre.size.width - re.size.width) / 2) + pre.origin.x;
	ori.y = pre.origin.y - re.size.height;

	[vKbdWin setFrameOrigin:ori];
}

void VKeyboard::need_update_window(PressedInfo_t *info, bool onoff)
{
	need_update_window_base(info, onoff);

	if (vKbdWin) {
		CocoaVKeyboardView *vw = (CocoaVKeyboardView *)[vKbdWin contentView];
		NSRect re = NSMakeRect(info->re.left, info->re.top, info->re.right - info->re.left, info->re.bottom - info->re.top);
		[vw copyBufferPart:re];
	}
}

void VKeyboard::update_window()
{
	if (!pSurface) return;

	if (vKbdWin) {
//		CocoaVKeyboardView *vw = (CocoaVKeyboardView *)[vKbdWin contentView];
//		[vw copyBuffer];
		if (gui->IsFullScreen()) {
			[vKbdWin displayIfNeeded];
			[vKbdWin display];
		}
	}
}

/// @note called by CocoaVKeyboard
void VKeyboard::adjust_window_size(double mag)
{
	if (!pSurface) return;

	NSSize sufSize;
	sufSize.width = pSurface->Width();
	sufSize.height = pSurface->Height();
	magnify_x = mag;
	magnify_y = mag;
	NSView *vw = [vKbdWin contentView];
	NSRect vre = [vw frame];
	NSRect wre = [vKbdWin frame];
	NSSize margin;
	margin.width = wre.size.width - vre.size.width;
	margin.height = wre.size.height - vre.size.height;
	wre.size.width = sufSize.width * magnify_x + margin.width;
	wre.size.height = sufSize.height * magnify_y + margin.height;
	[vKbdWin setFrame:wre display:YES];
}

/// @note called by CocoaVKeyboard
void VKeyboard::set_magnify(double magx, double magy)
{
	magnify_x = magx;
	magnify_y = magy;
}

} /* namespace Vkbd */
