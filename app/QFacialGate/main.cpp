#include <QtWidgets/QtWidgets>
#include <getopt.h>
#include "desktopview.h"

static void usage(const char *name)
{
	printf("Usage: %s options\n", name);
	printf("-h --help  Display this usage information.\n"
			"-f --face  Set face number.\n"
			"-r --refresh  Set refresh frames.\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	int faceCnt = 0;
	int refreshFrame = 0;
	int nextOption;

	const char* const shortOptions = "hf:r:";
	const struct option longOptions[] = {
		{"help", 0, NULL, 'h'},
		{"face", 1, NULL, 'f'},
		{"refresh", 1, NULL, 'r'},
	};

	do {
		nextOption = getopt_long(argc, argv, shortOptions, longOptions, NULL);
		switch (nextOption) {
		case 'f':
			faceCnt = atoi(optarg);
			break;
		case 'r':
			refreshFrame = atoi(optarg);
			break;
		case -1:
			break;
		default:
			usage(argv[0]);
			break;
		}
	} while (nextOption != -1);

	QApplication a(argc, argv);

	Q_INIT_RESOURCE(QFacialGate);

	DesktopView desktop(faceCnt, refreshFrame);
	desktop.show();

	return a.exec();
}
