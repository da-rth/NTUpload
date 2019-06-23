/**
 * @author denBot@github
 * @date 23/06/2019
 */
#include <iostream>
#include <X11/X.h>
#include <X11/Xlib.h>

using namespace std;

Window *getWindowList(Display *disp, unsigned long *len) {
    Atom prop = XInternAtom(disp,"_NET_CLIENT_LIST",False), type;
    int form;
    unsigned long remain;
    unsigned char *list;

    if (XGetWindowProperty(
        disp,XDefaultRootWindow(disp),prop,0,1024,False,33,
        &type,&form,len,&remain,&list) != Success) {
        return 0;
    }

    return (Window*)list;
}
char *getWindowName(Display *disp, Window win) {
    Atom prop = XInternAtom(disp,"WM_NAME",False), type;
    int form;
    unsigned long remain, len;
    unsigned char *list;


    if (XGetWindowProperty(disp,win,prop,0,1024,False,AnyPropertyType,
                           &type,&form,&len,&remain,&list) != Success) { // XA_STRING

        return NULL;
    }

    return (char*)list;
}

void print_windows() {
    int i;
    unsigned long len;
    Display *disp = XOpenDisplay(NULL);
    Window *list;
    char *name;

    list = (Window*)getWindowList(disp,&len);
    for (i=0;i<(int)len;i++) {
        name = getWindowName(disp,list[i]);
        cout << i << ": " << name << endl;
        free(name);
    }
}
