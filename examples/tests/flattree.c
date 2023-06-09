/*IupFlatTree Example in C
Creates a tree with some branches and leaves.
Two callbacks are registered: one deletes marked nodes when the Del key is pressed,
and the other, called when the right mouse button is pressed, opens a menu with options. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupkey.h"


static Ihandle* load_image_LogoTecgraf(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 108, 120, 143, 125, 132, 148, 178, 173, 133, 149, 178, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100, 110, 130, 48, 130, 147, 177, 254, 124, 139, 167, 254, 131, 147, 176, 137, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 115, 128, 153, 134, 142, 159, 191, 194, 47, 52, 61, 110, 114, 128, 154, 222, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 143, 172, 192, 140, 156, 188, 99, 65, 69, 76, 16, 97, 109, 131, 251, 129, 144, 172, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 131, 147, 175, 232, 140, 157, 188, 43, 0, 0, 0, 0, 100, 112, 134, 211, 126, 141, 169, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 72, 78, 88, 26, 48, 52, 57, 60, 135, 150, 178, 254, 108, 121, 145, 83, 105, 118, 142, 76, 106, 119, 143, 201, 118, 133, 159, 122, 117, 129, 152, 25, 168, 176, 190, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    118, 128, 145, 3, 104, 117, 140, 92, 114, 127, 152, 180, 131, 147, 177, 237, 133, 149, 178, 249, 38, 42, 50, 222, 137, 152, 180, 249, 126, 142, 170, 182, 114, 128, 154, 182, 104, 117, 140, 227, 95, 107, 128, 238, 83, 93, 112, 248, 84, 95, 113, 239, 104, 117, 141, 180, 115, 129, 155, 93, 127, 140, 165, 4,
    98, 109, 130, 153, 109, 123, 147, 254, 145, 163, 195, 153, 138, 154, 182, 56, 115, 123, 138, 5, 92, 99, 109, 35, 134, 149, 177, 230, 0, 0, 0, 0, 0, 0, 0, 0, 120, 133, 159, 143, 135, 151, 181, 115, 86, 89, 93, 5, 41, 45, 51, 54, 40, 45, 53, 150, 107, 120, 144, 254, 122, 137, 164, 154,
    51, 57, 66, 147, 83, 93, 112, 255, 108, 121, 145, 159, 113, 126, 151, 62, 123, 136, 159, 8, 87, 93, 103, 35, 125, 141, 169, 230, 0, 0, 0, 0, 0, 0, 0, 0, 129, 143, 169, 143, 140, 156, 184, 115, 134, 147, 172, 8, 124, 138, 165, 60, 124, 139, 167, 155, 131, 147, 177, 255, 131, 147, 176, 153,
    64, 68, 73, 2, 36, 39, 45, 86, 41, 46, 54, 173, 60, 67, 80, 232, 75, 84, 101, 251, 89, 100, 120, 228, 105, 118, 142, 250, 110, 123, 148, 187, 118, 132, 158, 187, 126, 141, 169, 229, 134, 149, 177, 239, 136, 152, 179, 250, 136, 152, 181, 234, 139, 156, 186, 175, 130, 145, 173, 90, 124, 134, 151, 3,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 71, 74, 79, 19, 60, 64, 73, 50, 92, 103, 124, 254, 86, 95, 111, 84, 90, 100, 117, 76, 126, 141, 168, 201, 113, 126, 150, 119, 99, 105, 117, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 93, 105, 125, 231, 135, 151, 181, 46, 0, 0, 0, 0, 137, 154, 184, 212, 123, 137, 164, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 74, 83, 98, 191, 133, 149, 179, 102, 111, 121, 139, 17, 134, 150, 180, 252, 126, 140, 166, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 43, 48, 57, 132, 121, 136, 164, 197, 121, 135, 161, 115, 130, 146, 175, 221, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 43, 47, 52, 46, 87, 98, 118, 254, 126, 142, 170, 254, 124, 139, 166, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 51, 57, 67, 118, 115, 128, 152, 170, 127, 140, 164, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Ihandle* image = IupImageRGBA(16, 16, imgdata);
  return image;
}

#define TEST_IMAGE_SIZE 16

static Ihandle* load_image_TestImage(void)
{
  unsigned char image_data_8[TEST_IMAGE_SIZE*TEST_IMAGE_SIZE] =
  {
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,
    5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,
    5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,
    5,0,0,0,1,1,1,1,2,2,2,2,0,0,0,5,
    5,0,0,0,1,1,1,1,2,2,2,2,0,0,0,5,
    5,0,0,0,1,1,1,1,2,2,2,2,0,0,0,5,
    5,0,0,0,1,1,1,1,2,2,2,2,0,0,0,5,
    5,0,0,0,3,3,3,3,4,4,4,4,0,0,0,5,
    5,0,0,0,3,3,3,3,4,4,4,4,0,0,0,5,
    5,0,0,0,3,3,3,3,4,4,4,4,0,0,0,5,
    5,0,0,0,3,3,3,3,4,4,4,4,0,0,0,5,
    5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,
    5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,
    5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  };

  Ihandle* image = IupImage(TEST_IMAGE_SIZE, TEST_IMAGE_SIZE, image_data_8);
  IupSetAttribute(image, "0", "BGCOLOR");
  IupSetAttribute(image, "1", "255 0 0");
  IupSetAttribute(image, "2", "0 255 0");
  IupSetAttribute(image, "3", "0 0 255");
  IupSetAttribute(image, "4", "255 255 255");
  IupSetAttribute(image, "5", "0 0 0");

  return image;
}

static int addleaf(void)
{
  Ihandle* tree = IupGetHandle("flattree");
  int id = IupGetInt(tree, "VALUE");
  IupSetAttributeId(tree, "ADDLEAF", id, "");
  return IUP_DEFAULT;
}

static int insertleaf(Ihandle *ih)
{
  Ihandle* tree = IupGetHandle("flattree");
  int id = IupGetInt(tree, "VALUE");
  IupSetAttributeId(tree, "INSERTLEAF", id, "");
  return IUP_DEFAULT;
}

static int addbranch(void)
{
  Ihandle* tree = IupGetHandle("flattree");
  int id = IupGetInt(tree, "VALUE");
  IupSetAttributeId(tree, "ADDBRANCH", id, "");
  return IUP_DEFAULT;
}

static int insertbranch(void)
{
  Ihandle* tree = IupGetHandle("flattree");
  int id = IupGetInt(tree, "VALUE");
  IupSetAttributeId(tree, "INSERTBRANCH", id, "");
  return IUP_DEFAULT;
}

static int togglestate(void)
{
  char *value;
  Ihandle* tree = IupGetHandle("flattree");
  int id = IupGetInt(tree, "VALUE");
  value = IupGetAttributeId(tree, "STATE", id);
  if (value)
  {
    if (strcmp(value, "EXPANDED") == 0)
      IupSetAttributeId(tree, "STATE", id, "COLLAPSED");
    else
      IupSetAttributeId(tree, "STATE", id, "EXPANDED");
  }
  return IUP_DEFAULT;
}

static int togglemarkmode(void)
{
  char *value;
  Ihandle* tree = IupGetHandle("flattree");
  value = IupGetAttribute(tree, "MARKMODE");
  if (strcmp(value, "SINGLE") == 0)
    IupSetAttribute(tree, "MARKMODE", "MULTIPLE");
  else
    IupSetAttribute(tree, "MARKMODE", "SINGLE");
  printf("MARKMODE=%s\n", IupGetAttribute(tree, "MARKMODE"));
  return IUP_DEFAULT;
}

static int text_cb(Ihandle* ih, int c, char *after)
{
  if (c == K_ESC)
    return IUP_CLOSE;

  if (c == K_CR)
  {
    Ihandle *tree = IupGetHandle("flattree");
    IupSetAttribute(tree, "NAME", after);
    return IUP_CLOSE;
  }

  return IUP_DEFAULT;
}

static int tips_cb(Ihandle* ih, int x, int y)
{
  printf("TIPS_CB(%d, %d)\n", x, y);
  return IUP_DEFAULT;
}

static int removenode(void)
{
  Ihandle* tree = IupGetHandle("flattree");
  IupSetAttribute(tree, "DELNODE", "SELECTED");
  return IUP_DEFAULT;
}

static int removechild(void)
{
  Ihandle* tree = IupGetHandle("flattree");
  IupSetAttribute(tree, "DELNODE", "CHILDREN");
  return IUP_DEFAULT;
}

static int removemarked(void)
{
  Ihandle* tree = IupGetHandle("flattree");
  IupSetAttribute(tree, "DELNODE", "MARKED");
  return IUP_DEFAULT;
}

static int removeall(void)
{
  Ihandle* tree = IupGetHandle("flattree");
  IupSetAttribute(tree, "DELNODE", "ALL");
  return IUP_DEFAULT;
}

static int setFont(void)
{
  Ihandle* tree = IupGetHandle("flattree");
  Ihandle* fontdlg = IupFontDlg();
  int id = IupGetInt(tree, "VALUE");

  char* font = IupGetAttributeId(tree, "TITLEFONT", id);

  IupSetStrAttribute(fontdlg, "VALUE", font);
  IupPopup(fontdlg, IUP_CENTER, IUP_CENTER);

  if (IupGetInt(fontdlg, "STATUS") == 1)
  {
    font = IupGetAttribute(fontdlg, "VALUE");
    IupSetStrAttributeId(tree, "TITLEFONT", id, font);
  }
  return IUP_DEFAULT;
}

static int expandall(void)
{
  Ihandle* tree = IupGetHandle("flattree");
  IupSetAttribute(tree, "EXPANDALL", "YES");
  return IUP_DEFAULT;
}

static int contractall(void)
{
  Ihandle* tree = IupGetHandle("flattree");
  IupSetAttribute(tree, "EXPANDALL", "NO");
  return IUP_DEFAULT;
}

static int renamenode(void)
{
  Ihandle* tree = IupGetHandle("flattree");
  IupSetAttribute(tree, "RENAME", "YES");
  return IUP_DEFAULT;
}

static int button_cb(Ihandle *ih, int but, int pressed, int x, int y, char* status)
{
  printf("FLAT_BUTTON_CB(but=%c (%d), x=%d, y=%d [%s]) - [id=%d]\n", (char)but, pressed, x, y, status, IupConvertXYToPos(ih, x, y));
  return IUP_DEFAULT;
}

static int motion_cb(Ihandle *ih, int x, int y, char* status)
{
  printf("MOTION_CB(x=%d, y=%d [%s]) - [id=%d]\n", x, y, status, IupConvertXYToPos(ih, x, y));
  return IUP_DEFAULT;
}

static int showrename_cb(Ihandle* ih, int id)
{
  (void)ih;
  printf("SHOWRENAME_CB(%d)\n", id);
  if (id == 6)
    return IUP_IGNORE;
  return IUP_DEFAULT;
}

static int togglevalue_cb(Ihandle* ih, int id, int status)
{
  (void)ih;
  printf("TOGGLEVALUE_CB(%d, %d)\n", id, status);
  return IUP_DEFAULT;
}

static int selection_cb(Ihandle *ih, int id, int status)
{
  (void)ih;
  printf("SELECTION_CB(id=%d, status=%d)\n", id, status);
  printf("    USERDATA=%s\n", IupGetAttributeId(ih, "USERDATA", id));
  printf("    TOTALCHILDCOUNT=%s\n", IupGetAttributeId(ih, "TOTALCHILDCOUNT", id));
  return IUP_DEFAULT;
}

static int multiselection_cb(Ihandle *ih, int* ids, int n)
{
  int i;
  (void)ih;
  printf("MULTISELECTION_CB(");
  for (i = 0; i < n; i++)
    printf("%d, ", ids[i]);
  printf("n=%d)\n", n);
  return IUP_DEFAULT;
}

static int multiunselection_cb(Ihandle *ih, int* ids, int n)
{
  int i;
  (void)ih;
  printf("MULTIUNSELECTION_CB(");
  for (i = 0; i < n; i++)
    printf("%d, ", ids[i]);
  printf("n=%d)\n", n);
  return IUP_DEFAULT;
}

static int executeleaf_cb(Ihandle* ih, int id)
{
  printf("EXECUTELEAF_CB (%d)\n", id);
  return IUP_DEFAULT;
}

static int rename_cb(Ihandle* ih, int id, char* title)
{
  printf("RENAME_CB (%d=%s)\n", id, title);
  if (strcmp(title, "fool") == 0)
    return IUP_IGNORE;
  return IUP_DEFAULT;
}

static int branchopen_cb(Ihandle* ih, int id)
{
  printf("BRANCHOPEN_CB (%d)\n", id);
  //  if (id == 6)
  //    return IUP_IGNORE;
  return IUP_DEFAULT;
}

static int branchclose_cb(Ihandle* ih, int id)
{
  printf("BRANCHCLOSE_CB (%d)\n", id);
  //  if (id == 6)
  //    return IUP_IGNORE;
  return IUP_DEFAULT;
}

static int noderemoved_cb(Ihandle* ih, void* data)
{
  printf("NODEREMOVED_CB(userdata=%p)\n", data);
  return IUP_DEFAULT;
}

static int dragdrop_cb(Ihandle* ih, int drag_id, int drop_id, int shift, int control)
{
  printf("DRAGDROP_CB (%d)->(%d) shift=%d ctrl=%d\n", drag_id, drop_id, shift, control);
  return IUP_CONTINUE;
  //  return IUP_DEFAULT;
}

static int getfocus_cb(Ihandle* ih)
{
  printf("GETFOCUS_CB()\n");
  return IUP_DEFAULT;
}

static int killfocus_cb(Ihandle* ih)
{
  printf("KILLFOCUS_CB()\n");
  return IUP_DEFAULT;
}

static int leavewindow_cb(Ihandle *ih)
{
  printf("LEAVEWINDOW_CB()\n");
  return IUP_DEFAULT;
}

static int enterwindow_cb(Ihandle* ih)
{
  printf("ENTERWINDOW_CB()\n");
  return IUP_DEFAULT;
}

char *iupKeyCodeToName(int code);

static int k_any_cb(Ihandle* ih, int c)
{
  if (c == K_DEL)
    IupSetAttribute(ih, "DELNODE", "MARKED");
  if (iup_isprint(c))
    printf("K_ANY(%s, %d = %s \'%c\')\n", IupGetAttribute(IupGetParent(IupGetParent(ih)), "TITLE"), c, iupKeyCodeToName(c), (char)c);
  else
    printf("K_ANY(%s, %d = %s)\n", IupGetAttribute(IupGetParent(IupGetParent(ih)), "TITLE"), c, iupKeyCodeToName(c));
  return IUP_CONTINUE;
}

static int help_cb(Ihandle* ih)
{
  printf("HELP_CB()\n");
  return IUP_DEFAULT;
}

static int selectnode(Ihandle* ih)
{
  Ihandle* tree = IupGetHandle("flattree");
  IupSetAttribute(tree, "VALUE", IupGetAttribute(ih, "TITLE"));
  return IUP_DEFAULT;
}

static int markednode(Ihandle* ih)
{
  Ihandle* tree = IupGetHandle("flattree");
  int id = IupGetInt(tree, "VALUE");
  IupSetAttributeId(tree, "MARKED", id, IupGetAttribute(ih, "TITLE"));
  return IUP_DEFAULT;
}

static int markstart(Ihandle* ih)
{
  Ihandle* tree = IupGetHandle("flattree");
  int id = IupGetInt(tree, "VALUE");
  IupSetInt(tree, "MARKSTART", id);
  return IUP_DEFAULT;
}

static int marknode(Ihandle* ih)
{
  Ihandle* tree = IupGetHandle("flattree");
  IupSetAttribute(tree, "MARK", IupGetAttribute(ih, "TITLE"));
  return IUP_DEFAULT;
}

static int nodeinfo(Ihandle* ih)
{
  char attr[50], *kind;
  Ihandle* dial = IupGetAttributeHandle(ih, "DIAL");
  Ihandle* tree = IupGetAttributeHandle(dial, "FLATTREE");
  int branch = 0, id = IupGetInt(tree, "VALUE");
  printf("\nTree Info:\n");
  printf("  TOTALCOUNT=%s\n", IupGetAttribute(tree, "COUNT"));
  if (id == -1)
    return IUP_DEFAULT;
  printf("Node Info:\n");
  printf("  ID=%d\n", id);
  printf("  TITLE=%s\n", IupGetAttribute(tree, "TITLE"));
  printf("  DEPTH=%s\n", IupGetAttribute(tree, "DEPTH"));
  sprintf(attr, "KIND%d", id);
  kind = IupGetAttribute(tree, "KIND");
  printf("  KIND=%s\n", kind);
  if (strcmp(kind, "BRANCH") == 0) branch = 1;
  if (branch)
    printf("  STATE=%s\n", IupGetAttribute(tree, "STATE"));
  printf("  IMAGE=%s\n", IupGetAttributeId(tree, "IMAGE", id));
  if (branch)
    printf("  IMAGEBRANCHEXPANDED=%s\n", IupGetAttribute(tree, "IMAGEBRANCHEXPANDED"));
  printf("  MARKED=%s\n", IupGetAttribute(tree, "MARKED"));
  printf("  COLOR=%s\n", IupGetAttributeId(tree, "COLOR", id));
  printf("  PARENT=%s\n", IupGetAttributeId(tree, "PARENT", id));
  printf("  CHILDCOUNT=%s\n", IupGetAttribute(tree, "CHILDCOUNT"));
  printf("  USERDATA=%s\n", IupGetAttributeId(tree, "USERDATA", id));
  return IUP_DEFAULT;
}

static int rightclick_cb(Ihandle* ih, int id)
{
  Ihandle *popup_menu;

  popup_menu = IupMenu(
    IupItem("Node Info", "nodeinfo"),
    IupItem("Rename Node", "renamenode"),
    IupSeparator(),
    IupItem("Add Leaf", "addleaf"),
    IupItem("Add Branch", "addbranch"),
    IupItem("Insert Leaf", "insertleaf"),
    IupItem("Insert Branch", "insertbranch"),
    IupItem("Remove Node", "removenode"),
    IupItem("Remove Children", "removechild"),
    IupItem("Remove Marked", "removemarked"),
    IupItem("Remove All", "removeall"),
    IupItem("Set Font", "setfont"),
    IupSeparator(),
    IupItem("Toggle State", "togglestate"),
    IupItem("Expand All", "expandall"),
    IupItem("Contract All", "contractall"),
    IupSubmenu("Focus (VALUE)", IupMenu(
      IupItem("ROOT", "selectnode"),
      IupItem("LAST", "selectnode"),
      IupItem("PGUP", "selectnode"),
      IupItem("PGDN", "selectnode"),
      IupItem("NEXT", "selectnode"),
      IupItem("PREVIOUS", "selectnode"),
      IupItem("CLEAR", "selectnode"),
      NULL)),
    IupItem("Toggle Mark Mode", "togglemarkmode"),
    IupSubmenu("Marked", IupMenu(
      IupItem("Yes", "markednode"),
      IupItem("No", "markednode"),
      NULL)),
    IupSubmenu("Mark (multiple)", IupMenu(
      IupItem("INVERT", "marknode"),
      IupItem("BLOCK", "marknode"),
      IupItem("CLEARALL", "marknode"),
      IupItem("MARKALL", "marknode"),
      IupItem("INVERTALL", "marknode"),
      IupSeparator(),
      IupItem("MARKSTART", "markstart"),
      NULL)),
    NULL);

  IupSetFunction("nodeinfo", (Icallback)nodeinfo);
  IupSetFunction("selectnode", (Icallback)selectnode);
  IupSetFunction("marknode", (Icallback)marknode);
  IupSetFunction("markednode", (Icallback)markednode);
  IupSetFunction("markstart", (Icallback)markstart);
  IupSetFunction("togglemarkmode", (Icallback)togglemarkmode);
  IupSetFunction("addleaf", (Icallback)addleaf);
  IupSetFunction("addbranch", (Icallback)addbranch);
  IupSetFunction("insertleaf", (Icallback)insertleaf);
  IupSetFunction("insertbranch", (Icallback)insertbranch);
  IupSetFunction("removenode", (Icallback)removenode);
  IupSetFunction("removechild", (Icallback)removechild);
  IupSetFunction("removemarked", (Icallback)removemarked);
  IupSetFunction("renamenode", (Icallback)renamenode);
  IupSetFunction("togglestate", (Icallback)togglestate);
  IupSetFunction("removeall", (Icallback)removeall);
  IupSetFunction("setfont", (Icallback)setFont);
  IupSetFunction("expandall", (Icallback)expandall);
  IupSetFunction("contractall", (Icallback)contractall);

  IupSetAttributeHandle(popup_menu, "DIAL", IupGetDialog(ih));

  //  sprintf(attr, "%d", id);
  //  IupSetAttribute(ih, "VALUE", attr);
  IupPopup(popup_menu, IUP_MOUSEPOS, IUP_MOUSEPOS);

  IupDestroy(popup_menu);

  return IUP_DEFAULT;
}

static int active(Ihandle *ih)
{
  Ihandle* tree = IupGetHandle("flattree");
  if (IupGetInt(tree, "ACTIVE"))
    IupSetAttribute(tree, "ACTIVE", "NO");
  else
    IupSetAttribute(tree, "ACTIVE", "YES");
  return IUP_DEFAULT;
}

static int next(Ihandle *ih)
{
  Ihandle* tree = IupGetHandle("flattree");
  IupSetAttribute(tree, "VALUE", "NEXT");
  return IUP_DEFAULT;
}

static int prev(Ihandle *ih)
{
  Ihandle* tree = IupGetHandle("flattree");
  IupSetAttribute(tree, "VALUE", "PREVIOUS");
  return IUP_DEFAULT;
}

static int show_tree_2(Ihandle *ih)
{
  Ihandle *dlg = IupGetHandle("dlg_2");
  IupShowXY(dlg, IUP_CENTER, IUP_CENTER); /* Displays the dlg */
  return IUP_DEFAULT;
}

static void init_tree_nodes(void)
{
  Ihandle* tree = IupGetHandle("flattree");

  //Ihandle *bimage = IupLoadImage("07d8c3e8ad46509c4c6.jpg");
  //IupSetAttributeHandle(tree, "BACKIMAGE", bimage);

//    IupSetAttribute(tree, "AUTOREDRAW", "No");
#if 0
  /* create from bottom to top */
  /* the current node is the ROOT */
  //IupSetAttribute(tree, "VALUE", "0");
  IupSetAttribute(tree, "TITLE", "Figures");  /* title of the root, id=0 */
  IupSetAttribute(tree, "ADDBRANCH", "3D");    /* 3D=1 */
  IupSetAttribute(tree, "ADDLEAF", "2D");    /* add to the root, so it will be before "3D", now 2D=1, 3D=2 */
  IupSetAttribute(tree, "ADDBRANCH", "parallelogram"); /* id=1 */
  IupSetAttribute(tree, "ADDLEAF1", "diamond");
  IupSetAttribute(tree, "ADDLEAF1", "square");
  IupSetAttribute(tree, "ADDBRANCH", "triangle");
  IupSetAttribute(tree, "ADDLEAF1", "scalenus");
  IupSetAttribute(tree, "ADDLEAF1", "isoceles");
  IupSetAttribute(tree, "ADDLEAF1", "equilateral");
  IupSetAttribute(tree, "ADDLEAF", "Other");
#else
  /* create from top to bottom */

  IupSetAttribute(tree, "ADDBRANCH-1", "Figures");
  IupSetAttribute(tree, "ADDLEAF0", "Other");     /* new id=1 */
  IupSetAttribute(tree, "ADDBRANCH1", "triangle");  /* new id=2 */
  IupSetAttribute(tree, "ADDLEAF2", "equilateral");  /* ... */
  IupSetAttribute(tree, "ADDLEAF3", "isoceles");
  IupSetAttribute(tree, "ADDLEAF4", "scalenus");
  IupSetAttribute(tree, "STATE2", "expanded");
  IupSetAttribute(tree, "INSERTBRANCH2", "parallelogram");  /* same depth as id=2, new id=6 */
  //IupSetAttribute(tree, "ADDBRANCH2", "parallelogram");  /* same depth as id=2, new id=6 */
  IupSetAttribute(tree, "ADDLEAF6", "square very long string at tree node");
  IupSetAttribute(tree, "ADDLEAF7", "diamond");
  IupSetAttribute(tree, "INSERTLEAF6", "2D");  /* new id=9 */
  IupSetAttribute(tree, "INSERTBRANCH9", "3D");
  if (IupGetInt(NULL, "UTF8MODE"))
    IupSetAttribute(tree, "INSERTBRANCH10", "Other (çãõáóé)");
  else
    IupSetAttribute(tree, "INSERTBRANCH10", "Other (������)");
  IupSetAttribute(tree, "ADDLEAF11", "Depth 1");
  IupSetAttribute(tree, "ADDBRANCH12", "Folder");
  IupSetAttribute(tree, "ADDLEAF13", "Depth 2");
#endif

  IupSetAttribute(tree, "TOGGLEVALUE2", "ON");
  IupSetAttribute(tree, "TOGGLEVALUE6", "ON");
  //IupSetAttribute(tree, "TOGGLEVALUE9", "NOTDEF");
  //IupSetAttribute(tree, "TOGGLEVALUE2", "OFF");
  //  IupSetAttribute(tree, "AUTOREDRAW", "Yes");
  IupSetAttribute(tree, "TOGGLEVISIBLE7", "No");
  //IupSetAttribute(tree, "EMPTYAS3STATE7", "Yes");
//  IupSetAttribute(tree, "NODEACTIVE5", "No");

  //IupSetAttribute(tree, "MARKED1", "Yes");
  //IupSetAttribute(tree, "MARKED8", "Yes");
  //IupSetAttribute(tree, "VALUE",  "6");

  IupSetAttribute(tree, "RASTERSIZE", NULL);   /* remove the minimum size limitation */
  IupSetAttribute(tree, "COLOR8", "92 92 255");
  //  IupSetAttribute(tree, "TITLEFONT8", "Courier, 14");
  IupSetAttribute(tree, "IMAGE7", "IMGEMPTY");
  IupSetAttributeHandle(tree, "IMAGE1", load_image_LogoTecgraf());
  IupSetAttributeHandle(tree, "IMAGE2", load_image_TestImage());
  //  IupSetAttribute(tree, "IMAGE6", IupGetAttribute(tree, "IMAGE8"));

//  IupSetAttribute(tree, "BACKCOLOR3", "0 255 0");
//  IupSetAttribute(tree, "BACKCOLOR4", "0 255 0");
  IupSetAttribute(tree, "BACKCOLOR5", "0 255 0");

  IupSetAttribute(tree, "COLOR4", "255 0 0");
  IupSetAttribute(tree, "COLOR5", "255 0 0");
//  IupSetAttribute(tree, "COLOR6", "255 0 0");

  IupSetAttribute(tree, "ITEMTIP6", "Node Tip");

  IupSetAttribute(tree, "TITLEFONTSTYLE3", "Bold");

  IupSetAttribute(tree, "USERDATA0", "0");
  IupSetAttribute(tree, "USERDATA1", "1");
  IupSetAttribute(tree, "USERDATA2", "2");
  IupSetAttribute(tree, "USERDATA3", "3");
  IupSetAttribute(tree, "USERDATA4", "4");
  IupSetAttribute(tree, "USERDATA5", "5");
  IupSetAttribute(tree, "USERDATA6", "6");
  IupSetAttribute(tree, "USERDATA7", "7");
  IupSetAttribute(tree, "USERDATA8", "8");
  IupSetAttribute(tree, "USERDATA9", "9");

  IupSetAttribute(tree, "EXTRATEXT2", "Extra Text 2");
  IupSetAttribute(tree, "EXTRATEXT3", "Very Long Extra Text 3");
  IupSetAttribute(tree, "EXTRATEXT4", "4");
  IupSetAttribute(tree, "USERDATA6", "Extra Text 6");
  IupSetAttribute(tree, "USERDATA7", "�a���");
}

static void init_tree_2_nodes(void)
{
  Ihandle* tree2 = IupGetHandle("flattree_2");

//  Ihandle *bimage = IupLoadImage("7d8c3e8ad46509c4c6.jpg");
//  IupSetAttributeHandle(tree2, "BACKIMAGE", bimage);

    //IupSetAttribute(tree2, "AUTOREDRAW", "No");
#if 0
  /* create from bottom to top */
  /* the current node is the ROOT */
  //IupSetAttribute(tree2, "VALUE", "0");
  IupSetAttribute(tree2, "TITLE", "Figures");  /* title of the root, id=0 */
  IupSetAttribute(tree2, "ADDBRANCH", "3D");    /* 3D=1 */
  IupSetAttribute(tree2, "ADDLEAF", "2D");    /* add to the root, so it will be before "3D", now 2D=1, 3D=2 */
  IupSetAttribute(tree2, "ADDBRANCH", "parallelogram"); /* id=1 */
  IupSetAttribute(tree2, "ADDLEAF1", "diamond");
  IupSetAttribute(tree2, "ADDLEAF1", "square");
  IupSetAttribute(tree2, "ADDBRANCH", "triangle");
  IupSetAttribute(tree2, "ADDLEAF1", "scalenus");
  IupSetAttribute(tree2, "ADDLEAF1", "isoceles");
  IupSetAttribute(tree2, "ADDLEAF1", "equilateral");
  IupSetAttribute(tree2, "ADDLEAF", "Other");
#else
  /* create from top to bottom */

  IupSetAttribute(tree2, "ADDBRANCH-1", "NFL");
  IupSetAttribute(tree2, "ADDBRANCH0", "AFC");  /* new id=2 */
  IupSetAttribute(tree2, "ADDBRANCH1", "EAST");  /* ... */
  IupSetAttribute(tree2, "ADDLEAF2", "Patriots");
  IupSetAttribute(tree2, "ADDLEAF3", "Bills");
  IupSetAttribute(tree2, "ADDLEAF4", "Jets");
  IupSetAttribute(tree2, "ADDLEAF5", "Dolphins");  /* same depth as id=2, new id=6 */
  IupSetAttribute(tree2, "INSERTBRANCH2", "North");  /* same depth as id=2, new id=6 */
  IupSetAttribute(tree2, "ADDLEAF7", "Ravens");
  IupSetAttribute(tree2, "ADDLEAF8", "Steelers");
  IupSetAttribute(tree2, "ADDLEAF9", "Browns");
  IupSetAttribute(tree2, "ADDLEAF10", "Bengals");  /* same depth as id=2, new id=6 */
  IupSetAttribute(tree2, "INSERTBRANCH7", "South");  /* same depth as id=2, new id=6 */
  IupSetAttribute(tree2, "ADDLEAF12", "Colts");
  IupSetAttribute(tree2, "ADDLEAF13", "Texans");
  IupSetAttribute(tree2, "ADDLEAF14", "Titans");
  IupSetAttribute(tree2, "ADDLEAF15", "Jaguars");  /* same depth as id=2, new id=6 */
  IupSetAttribute(tree2, "INSERTBRANCH12", "West");  /* same depth as id=2, new id=6 */
  IupSetAttribute(tree2, "ADDLEAF17", "Chiefs");
  IupSetAttribute(tree2, "ADDLEAF18", "Raiders");
  IupSetAttribute(tree2, "ADDLEAF19", "Chargers");
  IupSetAttribute(tree2, "ADDLEAF20", "Broncos");  /* same depth as id=2, new id=6 */
#endif

  IupSetAttribute(tree2, "TOGGLEVALUE2", "ON");
  IupSetAttribute(tree2, "TOGGLEVALUE6", "ON");
  //IupSetAttribute(tree2, "TOGGLEVALUE9", "NOTDEF");
  //IupSetAttribute(tree2, "TOGGLEVALUE2", "OFF");
  //  IupSetAttribute(tree2, "AUTOREDRAW", "Yes");
  //IupSetAttribute(tree2, "TOGGLEVISIBLE7", "No");
  //IupSetAttribute(tree2, "EMPTYAS3STATE7", "Yes");
//  IupSetAttribute(tree2, "NODEACTIVE5", "No");

  //IupSetAttribute(tree2, "MARKED1", "Yes");
  //IupSetAttribute(tree2, "MARKED8", "Yes");
  //IupSetAttribute(tree2, "VALUE",  "6");

  IupSetAttribute(tree2, "RASTERSIZE", NULL);   /* remove the minimum size limitation */
  IupSetAttribute(tree2, "COLOR8", "92 92 255");
  //  IupSetAttribute(tree2, "TITLEFONT8", "Courier, 14");
//  IupSetAttribute(tree2, "IMAGE3", "IUP_pat");
//  IupSetAttribute(tree2, "IMAGE4", "IUP_bil");
//  IupSetAttribute(tree2, "IMAGE5", "IUP_jet");
//  IupSetAttribute(tree2, "IMAGE6", "IUP_dol");
  IupSetAttributeHandle(tree2, "IMAGE8", load_image_LogoTecgraf());
  IupSetAttributeHandle(tree2, "IMAGE7", load_image_TestImage());
  //  IupSetAttribute(tree2, "IMAGE6", IupGetAttribute(tree2, "IMAGE8"));

//  IupSetAttribute(tree2, "ITEMBGCOLOR3", "0 255 0");
//  IupSetAttribute(tree2, "ITEMBGCOLOR4", "0 255 0");
  IupSetAttribute(tree2, "ITEMBGCOLOR5", "0 255 0");

  IupSetAttribute(tree2, "ITEMFGCOLOR4", "255 0 0");
  IupSetAttribute(tree2, "ITEMFGCOLOR5", "255 0 0");
//  IupSetAttribute(tree2, "ITEMFGCOLOR6", "255 0 0");

  IupSetAttribute(tree2, "ITEMFONTSTYLE3", "Bold");

  IupSetAttribute(tree2, "USERDATA0", "0");
  IupSetAttribute(tree2, "USERDATA1", "1");
  IupSetAttribute(tree2, "USERDATA2", "2");
  IupSetAttribute(tree2, "USERDATA3", "3");
  IupSetAttribute(tree2, "USERDATA4", "4");
  IupSetAttribute(tree2, "USERDATA5", "5");
  IupSetAttribute(tree2, "USERDATA6", "6");
  IupSetAttribute(tree2, "USERDATA7", "7");
  IupSetAttribute(tree2, "USERDATA8", "8");
  IupSetAttribute(tree2, "USERDATA9", "9");
}

/* Initializes IupFlatTree and registers callbacks */
static Ihandle *init_tree_2(void)
{
  Ihandle* tree2 = IupFlatTree();

  IupSetCallback(tree2, "EXECUTELEAF_CB", (Icallback)executeleaf_cb);
  IupSetCallback(tree2, "RENAME_CB", (Icallback)rename_cb);
  IupSetCallback(tree2, "BRANCHCLOSE_CB", (Icallback)branchclose_cb);
  IupSetCallback(tree2, "BRANCHOPEN_CB", (Icallback)branchopen_cb);
  IupSetCallback(tree2, "DRAGDROP_CB", (Icallback)dragdrop_cb);
  IupSetCallback(tree2, "RIGHTCLICK_CB", (Icallback)rightclick_cb);
  //IupSetCallback(tree2, "K_ANY", (Icallback)k_any_cb);
  IupSetCallback(tree2, "SHOWRENAME_CB", (Icallback)showrename_cb);
  IupSetCallback(tree2, "SELECTION_CB", (Icallback)selection_cb);
  //  IupSetCallback(tree2, "MULTISELECTION_CB", (Icallback) multiselection_cb);
  //  IupSetCallback(tree2, "MULTIUNSELECTION_CB", (Icallback) multiunselection_cb);
  //IupSetCallback(tree2, "GETFOCUS_CB", (Icallback)getfocus_cb);
  //IupSetCallback(tree2, "KILLFOCUS_CB", (Icallback)killfocus_cb);
  //IupSetCallback(tree2, "ENTERWINDOW_CB", (Icallback) enterwindow_cb);
  //IupSetCallback(tree2, "LEAVEWINDOW_CB", (Icallback)leavewindow_cb);
  //IupSetCallback(tree2, "FLAT_BUTTON_CB", (Icallback)button_cb);
  //IupSetCallback(tree2, "FLAT_MOTION_CB",    (Icallback)motion_cb);
  IupSetCallback(tree2, "NODEREMOVED_CB", (Icallback)noderemoved_cb);
  IupSetCallback(tree2, "TOGGLEVALUE_CB", (Icallback)togglevalue_cb);
  //  IupSetCallback(tree2, "TIPS_CB", (Icallback)tips_cb);
  //IupSetAttribute(tree2, "FLATSCROLLBAR", "Yes");

//  IupSetAttribute(tree2, "FITTOBACKIMAGE", "YES");

  //IupSetAttribute(tree2, "EXPAND", "YES");

  IupSetCallback(tree2, "HELP_CB", (Icallback)help_cb);

  IupSetAttribute(tree2, "DRAGDROPTREE", "Yes");
  IupSetAttribute(tree2, "DRAGSOURCE", "YES");
  IupSetAttribute(tree2, "DRAGSOURCEMOVE", "YES");
  IupSetAttribute(tree2, "DRAGTYPES", "NODETREE");

//  IupSetAttribute(tree2, "DROPFILESTARGET", "YES");


  //  IupSetAttribute(tree2, "FONT", "COURIER_NORMAL_14");
  //  IupSetAttribute(tree2, "FONTSIZE", "36");
    //  IupSetAttribute(tree2, "FGCOLOR", "255 0 0");
  //  IupSetAttribute(tree2, "SPACING",   "10");
  //  IupSetAttribute(tree2, "BGCOLOR", "255 255 255");
  //  IupSetAttribute(tree2, "BGCOLOR", "128 0 255");

  IupSetAttribute(tree2, "MARKMODE",     "MULTIPLE");
  IupSetAttribute(tree2, "SHOWRENAME", "YES");
  //IupSetAttribute(tree2, "SHOWTOGGLE", "YES");
  IupSetAttribute(tree2, "MARKWHENTOGGLE", "YES");
  IupSetAttribute(tree2, "SHOWTOGGLE", "3STATE");
  //  IupSetAttribute(tree2, "DROPEQUALDRAG", "YES");

  IupSetAttribute(tree2, "ADDEXPANDED", "YES");
  //  IupSetAttribute(tree2, "HIDELINES",    "YES");
  //  IupSetAttribute(tree2, "HIDEBUTTONS",    "YES");
  //  IupSetAttribute(tree2, "INDENTATION",   "40");
  //  IupSetAttribute(tree2, "CANFOCUS", "NO");
  IupSetAttribute(tree2, "TIP", "Tree Tip");
  //  IupSetAttribute(tree2, "INFOTIP", "No");
    //  IupSetAttribute(tree2, "TIPBGCOLOR", "255 128 128");
  //  IupSetAttribute(tree2, "TIPFGCOLOR", "0 92 255");
  //  IupSetAttribute(tree2, "HLCOLOR", "240 116 64");

  //  IupSetAttribute(tree2, "IMAGELEAF", "IMGEMPTY");
  //  IupSetAttribute(tree2, "IMAGEBRANCHCOLLAPSED", "IMGEMPTY");
  //  IupSetAttribute(tree2, "IMAGEBRANCHEXPANDED", "IMGEMPTY");

    // Windows Only  
  //  IupSetAttribute(tree2, "TIPBALLOON", "YES");
  //  IupSetAttribute(tree2, "TIPBALLOONTITLE", "Tip Title");
  //  IupSetAttribute(tree2, "TIPBALLOONTITLEICON", "2");
  IupSetAttribute(tree2, "VISIBLECOLUMNS", "10");
  IupSetAttribute(tree2, "VISIBLELINES", "10");

  IupSetHandle("flattree_2", tree2);

  return tree2;
}

/* Initializes IupFlatTree and registers callbacks */
static Ihandle *init_tree(void)
{
  Ihandle* tree = IupFlatTree();

  IupSetCallback(tree, "EXECUTELEAF_CB", (Icallback)executeleaf_cb);
  IupSetCallback(tree, "RENAME_CB", (Icallback)rename_cb);
  IupSetCallback(tree, "BRANCHCLOSE_CB", (Icallback)branchclose_cb);
  IupSetCallback(tree, "BRANCHOPEN_CB", (Icallback)branchopen_cb);
  IupSetCallback(tree, "DRAGDROP_CB", (Icallback)dragdrop_cb);
  IupSetCallback(tree, "RIGHTCLICK_CB", (Icallback)rightclick_cb);
//  IupSetCallback(tree, "K_ANY", (Icallback)k_any_cb);
  IupSetCallback(tree, "SHOWRENAME_CB", (Icallback)showrename_cb);
  IupSetCallback(tree, "SELECTION_CB", (Icallback)selection_cb);
  //IupSetCallback(tree, "MULTISELECTION_CB", (Icallback) multiselection_cb);
  //  IupSetCallback(tree, "MULTIUNSELECTION_CB", (Icallback) multiunselection_cb);
  //IupSetCallback(tree, "GETFOCUS_CB", (Icallback)getfocus_cb);
  //IupSetCallback(tree, "KILLFOCUS_CB", (Icallback)killfocus_cb);
  //IupSetCallback(tree, "ENTERWINDOW_CB", (Icallback) enterwindow_cb);
  //IupSetCallback(tree, "LEAVEWINDOW_CB", (Icallback)leavewindow_cb);
  //IupSetCallback(tree, "FLAT_BUTTON_CB", (Icallback)button_cb);
  //IupSetCallback(tree, "FLAT_MOTION_CB",    (Icallback)motion_cb);
  IupSetCallback(tree, "NODEREMOVED_CB", (Icallback)noderemoved_cb);
  IupSetCallback(tree, "TOGGLEVALUE_CB", (Icallback)togglevalue_cb);
  //  IupSetCallback(tree, "TIPS_CB", (Icallback)tips_cb);
//  IupSetAttribute(tree, "FLATSCROLLBAR", "Yes");
  IupSetAttribute(tree, "EXTRATEXTWIDTH", "300");

  //IupSetAttribute(tree, "FITTOBACKIMAGE", "YES");
//  IupSetAttribute(tree, "ADDEXPANDED", "NO");
//  IupSetAttribute(tree, "EXPAND", "YES");

  IupSetCallback(tree, "HELP_CB", (Icallback)help_cb);

//  IupSetAttribute(tree, "DRAGDROPTREE", "Yes");
//  IupSetAttribute(tree, "DROPTARGET", "YES");
//  IupSetAttribute(tree, "DROPTYPES", "NODETREE");
  IupSetAttribute(tree, "SHOWDRAGDROP", "YES");

  //  IupSetAttribute(tree, "FONT", "COURIER_NORMAL_14");
  //  IupSetAttribute(tree, "FONTSIZE", "36");
    //  IupSetAttribute(tree, "FGCOLOR", "255 0 0");
//    IupSetAttribute(tree, "SPACING",   "50");
  //  IupSetAttribute(tree, "BGCOLOR", "255 255 255");
  //  IupSetAttribute(tree, "BGCOLOR", "128 0 255");

//  IupSetAttribute(tree, "MARKMODE",     "MULTIPLE");
  IupSetAttribute(tree, "SHOWRENAME", "YES");
//  IupSetAttribute(tree, "SHOWTOGGLE", "YES");
//  IupSetAttribute(tree, "MARKWHENTOGGLE", "YES");
//  IupSetAttribute(tree, "SHOWTOGGLE", "3STATE");
  //  IupSetAttribute(tree, "DROPEQUALDRAG", "YES");

  //  IupSetAttribute(tree, "HIDELINES",    "YES");
  //  IupSetAttribute(tree, "HIDEBUTTONS",    "YES");
  //  IupSetAttribute(tree, "INDENTATION",   "40");
  //  IupSetAttribute(tree, "CANFOCUS", "NO");
  IupSetAttribute(tree, "TIP", "Tree Tip");
  //  IupSetAttribute(tree, "INFOTIP", "No");
    //  IupSetAttribute(tree, "TIPBGCOLOR", "255 128 128");
  //  IupSetAttribute(tree, "TIPFGCOLOR", "0 92 255");
  //  IupSetAttribute(tree, "HLCOLOR", "240 116 64");

  //  IupSetAttribute(tree, "IMAGELEAF", "IMGEMPTY");
  //  IupSetAttribute(tree, "IMAGEBRANCHCOLLAPSED", "IMGEMPTY");
  //  IupSetAttribute(tree, "IMAGEBRANCHEXPANDED", "IMGEMPTY");

  IupSetAttribute(tree, "VISIBLECOLUMNS", "40");
  IupSetAttribute(tree, "VISIBLELINES", "60");

  IupSetHandle("flattree", tree);

  return tree;
}

/* Initializes the dlg */
static void init_dlg(void)
{
  Ihandle* butactv, *butnext, *butprev, *butmenu, *buttree;
  Ihandle* tree = init_tree();
  Ihandle* box = IupHbox(tree, IupVbox(butactv = IupButton("Active", NULL),
    butnext = IupButton("Next", NULL),
    butprev = IupButton("Prev", NULL),
    butmenu = IupButton("Menu", NULL),
    buttree = IupButton("Tree", NULL),
    NULL), NULL);
  Ihandle* dlg = IupDialog(box);
  IupSetAttribute(dlg, "TITLE", "IupFlatTree");
  IupSetAttribute(dlg, "SIZE", "400x400");
  IupSetAttribute(box, "MARGIN", "10x10");
  IupSetAttribute(box, "GAP", "10");
  //IupSetAttribute(box, "BGCOLOR", "92 92 255");
  //IupSetAttribute(dlg, "BGCOLOR", "92 92 255");
  //IupSetAttribute(dlg, "BACKGROUND", "200 10 80");
  //IupSetAttribute(dlg, "BGCOLOR", "173 177 194");  // Motif BGCOLOR for documentation
  IupSetCallback(butactv, "ACTION", active);
  IupSetCallback(butnext, "ACTION", next);
  IupSetCallback(butprev, "ACTION", prev);
  IupSetCallback(buttree, "ACTION", show_tree_2);
  IupSetCallback(butmenu, "ACTION", (Icallback)rightclick_cb);

  IupSetAttributeHandle(dlg, "FLATTREE", tree);

  IupSetHandle("dlg", dlg);
}

/* Initializes the dlg */
static void init_dlg_2(void)
{
  Ihandle* tree = init_tree_2();
  Ihandle* box = IupHbox(tree, NULL);
  Ihandle* dlg = IupDialog(box);
  IupSetAttribute(dlg, "TITLE", "IupFlatTree");
  IupSetAttribute(dlg, "SIZE", "400x400");
  IupSetAttribute(box, "MARGIN", "10x10");
  IupSetAttribute(box, "GAP", "10");
  //IupSetAttribute(box, "BGCOLOR", "92 92 255");
  //IupSetAttribute(dlg, "BGCOLOR", "92 92 255");
  //IupSetAttribute(dlg, "BACKGROUND", "200 10 80");
  //IupSetAttribute(dlg, "BGCOLOR", "173 177 194");  // Motif BGCOLOR for documentation

  IupSetAttributeHandle(dlg, "FLATTREE", tree);

  IupSetHandle("dlg_2", dlg);
}

void FlatTreeTest(void)
{
  Ihandle* dlg;

  init_dlg();                             /* Initializes the dlg */
  init_dlg_2();                             /* Initializes the dlg */
  dlg = IupGetHandle("dlg");              /* Retrieves the dlg handle */
  init_tree_nodes();                  /* Initializes attributes, can be done here or anywhere */
  init_tree_2_nodes();                  /* Initializes attributes, can be done here or anywhere */
  IupShowXY(dlg, IUP_CENTER, IUP_CENTER); /* Displays the dlg */
  IupSetAttribute(dlg, "USERSIZE", NULL);
}

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  TreeTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
