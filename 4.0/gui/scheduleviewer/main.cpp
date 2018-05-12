#include <QtGui>
#include <QApplication>
#include <viewer.h>

static bool debug = 0;

void msgHandler(QtMsgType type, const char *msg)
{
    switch (type) {
        case QtDebugMsg:
            if(debug){
                fprintf(stderr, "%s", msg);
            }
            break;
        case QtWarningMsg:
            fprintf(stderr, "Warning: %s", msg);
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s", msg);
            if(debug){
              fprintf(stderr, "\nAborting, because of debug mode\n");
              abort();
            }
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s\nAborting...", msg);
            abort();
    }
}

int main(int argc, char *argv[])
{
    int i;
    qInstallMsgHandler(msgHandler);
    QApplication a(argc, argv);
    char *schedule_path = NULL;
    char *pipeline_path = NULL;

    for(i = 1; i < argc; i++){
        if(strncmp("-d", argv[i], 2) == 0){
            debug = 1;
        }
        else if(strncmp("-p", argv[i], 2)==0 ){
          i++;
          if(i < argc)
            pipeline_path = argv[i];
        }
        else{
            schedule_path = argv[i];
        }
    }
    Viewer w(schedule_path, pipeline_path);
    w.show();
    return a.exec();
}
