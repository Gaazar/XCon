#include "FlameUI.h"
#include "Frame.h"
#include "Label.h"
#include "Button.h"
#include "Image.h"
#include "Scroller.h"
#include "ScrollView.h"
#include "CheckBox.h"
#include "Toggle.h"
#include "RadioButton.h"
#include "TextEditor.h"

#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <thread>


#include "LinearPlacer.h"
#include "SectionBar.h"
#include "SheetView.h"
#include "MenuFrame.h"
#include "DockProvider.h"
#include "SeperatorHandle.h"
#include "ImGuiCanvas.h"
#include "VideoPlayer.h"
#include "XInputCehcker.h"
#include "Chart.h"
#include "XInput.h"
#include "MenuBar.h"

#include "Transmission.h"
#include "json.hpp"
#include "Frames.h"
#include "global.h"
#include "Dropdown.h"
#include "Math3Df.h"
#include "AttitudeMeeter.h"

int CalibratedValue(int maxv, int minv, int zerov, float normv);