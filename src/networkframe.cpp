#include "networkframe.h"
#include "mainframe.h"
#include "config.h"

NetworkFrame::NetworkFrame(MainFrame *frame):
	jgui::Frame(__L->GetParam("networkframe.title"), 480, 240, 960, 400)
{
	// SetLayout(_info_layout = new jgui::GridLayout(1, 2));

	_address_label = new jgui::Label(__L->GetParam("networkframe.address_label"), _insets.left, _insets.top+8, 320, DEFAULT_COMPONENT_HEIGHT);
	_address = new jgui::ComboBox(_insets.left+320+8, _insets.top+8, 960-320-8-_insets.left-_insets.right, DEFAULT_COMPONENT_HEIGHT);

	FILE * fp = popen("ifconfig | grep 'inet ' | awk '{ print $2 }' | awk -F: '{ print $2 }' | grep -v '127.0.0'", "r");
	// FILE * fp = popen("ifconfig | grep 'inet ' | awk '{ print $2 }' | awk -F: '{ print $2 }'", "r");

	if (fp != NULL) {
		char *p = NULL, *e;
		size_t n;

		while ((getline(&p, &n, fp) > 0) && p) {
			_address->AddTextItem(std::string(p));
		}
	}

	pclose(fp);

	if (_address->GetItemsSize() < 2) {
		_address->SetVisibleItems(1);
	}

	Add(_address_label);
	Add(_address);
	
	// Add(_address_label, jgui::JBLA_WEST);
	// Add(_address, jgui::JBLA_CENTER);
	
	_address->RequestFocus();
}

NetworkFrame::~NetworkFrame()
{
	delete _address_label;
	delete _address;
	// delete _info_layout;
}

