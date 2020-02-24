#ifndef IMAGEENGINEAPI_H
#define IMAGEENGINEAPI_H

#include <QObject>
#include <QMap>
#include <QUrl>
#include "imageenginethread.h"
#include "thumbnail/thumbnaildelegate.h"

const int Number_Of_Displays_Per_Time = 100;


class ImageEngineApi: public QObject
{
    Q_OBJECT
public:
    static ImageEngineApi *instance(QObject *parent = nullptr);
    bool insertImage(QString imagepath, QString remainDay);
    bool removeImage(QString imagepath);
    bool getImageData(QString imagepath, ImageDataSt &data);
    bool updateImageDataPixmap(QString imagepath, QPixmap &pix);
    bool reQuestImageData(QString imagepath, ImageEngineObject *obj, bool needcache = true);
    bool imageNeedReload(QString imagepath);
    bool ImportImagesFromFileList(QStringList files, QString albumname, ImageEngineImportObject *obj, bool bdialogselect = false);
    bool ImportImagesFromUrlList(QList<QUrl> files, QString albumname, ImageEngineImportObject *obj, bool bdialogselect = false);
    bool loadImagesFromLocal(QStringList files, ImageEngineObject *obj, bool needcheck = true);
    bool loadImagesFromLocal(DBImgInfoList files, ImageEngineObject *obj, bool needcheck = true);
    bool loadImagesFromTrash(DBImgInfoList files, ImageEngineObject *obj);
    bool loadImagesFromDB(ThumbnailDelegate::DelegateType type, ImageEngineObject *obj, QString name = "");
    bool getImageFilesFromMount(QString mountname, QString path, ImageMountGetPathsObject *obj);
    bool importImageFilesFromMount(QString albumname, QStringList paths, ImageMountImportPathsObject *obj);
private slots:
    void sltImageLoaded(void *imgobject, QString path, ImageDataSt &data);
    void sltInsert(QString imagepath, QString remainDay);
    void sltImageLocalLoaded(void *imgobject, QStringList &filelist);
    void sltImageDBLoaded(void *imgobject, QStringList &filelist);
    void sltImageFilesGeted(void *imgobject, QStringList &filelist, QString path);
    void sltAborted(QString path);
    void sltImageFilesImported(void *imgobject, QStringList &filelist);
private:
    ImageEngineApi(QObject *parent = nullptr);
    QMap<QString, ImageDataSt>m_AllImageData;
    QThreadPool m_qtpool;
    static ImageEngineApi *s_ImageEngine;
};

#endif // IMAGEENGINEAPI_H
