#include "networkframe.h"
#include "mainframe.h"
#include "config.h"

NetworkFrame::NetworkFrame(MainFrame *frame):
	jgui::Frame(__L->GetParam("networkframe.title"))
{
	// SetLayout(_info_layout = new jgui::GridLayout(1, 2));
	
	SetBounds((_size.width-_size.width*0.60)/2, _size.height*0.10, _size.width*0.60, _size.height*0.80);

	FILE * fp = popen("ifconfig | grep 'inet ' | awk '{ print $2 }' | awk -F: '{ print $2 }' | grep -v '127.0.0'", "r");
	// FILE * fp = popen("ifconfig | grep 'inet ' | awk '{ print $2 }' | awk -F: '{ print $2 }'", "r");

	if (fp != NULL) {
		char *p = NULL, *e;
		size_t n;
		int i = 0;

		while ((getline(&p, &n, fp) > 0) && p) {
			Add(new jgui::Label(__L->GetParam("networkframe.address_label"), _insets.left, _insets.top+8+i*(DEFAULT_COMPONENT_HEIGHT+8), 240, DEFAULT_COMPONENT_HEIGHT));
			Add(new jgui::Label(p, _insets.left+240+8, _insets.top+8+i*(DEFAULT_COMPONENT_HEIGHT+8), _size.width-240-8-_insets.left-_insets.right, DEFAULT_COMPONENT_HEIGHT));
		}
	}

	pclose(fp);
}

NetworkFrame::~NetworkFrame()
{
	std::vector<jgui::Component *> cmps = GetComponents();

	RemoveAll();

	for (std::vector<jgui::Component *>::iterator i=cmps.begin(); i!=cmps.end(); i++) {
		jgui::Component *cmp = *i;

		delete cmp;
	}
}

