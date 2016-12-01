
#include "main.h"

AppParameters *g_appParams;

void main()
{
	g_appParams = new AppParameters();

	App app;
	app.go();
	
	cout << "Done!" << endl;
	cin.get();
}
