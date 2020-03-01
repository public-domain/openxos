
#define WP_STYLE        1	// Style number
#define WP_BITS1        2	// Window bits 1
#define WP_BITS2        3	// Window bits 2
#define WP_BITS3        4	// Window bits 3
#define WP_BITS4        5	// Window bits 4

#define WP_TEXTSIZE     6	// Text size
#define WP_TEXTWIDTH    7	// Text width
#define WP_TEXTFONT     8	// Text font
#define WP_TEXTATTR     9	// Text attributes
#define WP_ZORDER       10	// Z-order
#define WP_LEFTMARGIN   11	// Left margin
#define WP_RIGHTMARGIN  12	// Right margin
#define WP_TOPMARGIN    13	// Top margin
#define WP_BTMMARGIN    14	// Bottom margin
#define WP_LINESPACE    15	// Line spacing
#define WP_MAXSPACE     16	// Maximum line spacing
#define WP_LEADING      17	// Leading
#define WP_PARSPACE     18	// Paragraph spacing

#define WP_TEXTCOLORN   20	// Normal text color
#define WP_TEXTCOLORA   21	// Alternate text color

#define WP_BGCOLORN     24	// Normal background color
#define WP_BGCOLORA     25	// Alternate background color

#define WP_HILCOLORN    28	// Normal highlight color
#define WP_HILCOLORA    29	// Alternate highlight color
#define WP_SHDCOLORN    30	// Normal shadow color
#define WP_SHDCOLORA    31	// Alternate shadow color
#define WP_BRDCOLOR     32	// Border color
#define WP_BRDWIDTHO    33	// Outer border width
#define WP_BRDWIDTHC    34	// Center border width
#define WP_BRDWIDTHI    35	// Inner border width
#define WP_BRDBITS      36	// Border bits
#define WP_RADIUSP      37	// Corner radius (per-cent)
#define WP_RADIUSA      38	// Corner radius (absolute)
#define WP_ORIENT       39	// Orientation
#define WP_POFFSET      40	// Position offset for pressed button
#define WP_ACTWINDOW    41	// Activation window
#define WP_ACTFUNC      42	// Activation function
#define WP_MAXVALUE     43	// Maximum value
#define WP_MINVALUE     44	// Minimum value
#define WP_INC1VALUE    45	// First increment/decrement value
#define WP_INC2VALUE    46	// Second increment/decrement value
#define WP_INCSIZES     47	// Increment/decrement button sizes
#define WP_SBSIZE       48	// Scroll bar sizes
#define WP_SBBUTSIZE    49	// Scroll bar button sizes
#define WP_SBTEXTCOLORN 50	// Scroll bar normal text color
#define WP_SBTEXTCOLORA 51	// Scroll bar alternate text color
#define WP_SBBGCOLORN   52	// Scroll bar button normal background color
#define WP_SBBGCOLORA   53	// Scroll bar button alternate background color
#define WP_SBBARCOLOR   54	// Scroll bar bar background color
#define WP_SBHILCOLORN  55	// Scroll bar normal highlight color
#define WP_SBHILCOLORA  56	// Scroll bar alternate highlight color
#define WP_SBSHDCOLORN  57	// Scroll bar normal shadow color
#define WP_SBSHDCOLORA  58	// Scroll bar alternate shadow color
#define WP_SBSHDWIDTH   59	// Scroll bar shading widths

// Define bits for the value of WP_BITS1

#define WB1_FOCUS    0x00000001	// Force focus to window

// Define bits for the value of WP_BITS2

#define WB2_MODAL    0x00000400	//Modal window
#define WB2_MENULIST 0x00000100	// List window acts like a menu list
#define WB2_NOSHWFOC 0x00000080	// Window does not indicate input focus
#define WB2_NOFOCUS  0x00000040	// Window cannot receive input focus
#define WB2_MOVEABLE 0x00000020	// Window can be resized with the mouse
#define WB2_SIZEABLE 0x00000010	// Window can be moved with the mouse
#define WB2_ZORDER   0x00000008	// Window's z-order can be changed by clicking

// Define bits for the value of WP_BITS3

#define WB3_HORSB   0x00000200	// Window has a horizontal scroll bar
#define WB3_VERSB   0x00000100	// Window has a vertical scroll bar

#define WB3_RECESSO 0x00000008	// Recess outer border edge
#define WB3_RECESSI 0x00000004	// Recess inner border edge
#define WB3_LINEO   0x00000002	// Draw 1 pixel line outside border
#define WB3_LINEI   0x00000001	// Draw 1 pixel line inside border

// Define bits for the value of WP_BITS4
