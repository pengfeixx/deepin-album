#include "thumbnaillistview.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "controller/signalmanager.h"
#include "utils/snifferimageformat.h"
#include "dialogs/imgdeletedialog.h"
#include <QDebug>
#include <QImageReader>
#include <QFileInfo>
#include "timelinelist.h"

namespace {
const int ITEM_SPACING = 5;
const int BASE_HEIGHT = 100;
const int LEFT_MARGIN = 12;
const int RIGHT_MARGIN = 8;

const QString IMAGE_DEFAULTTYPE = "All pics";
const QString SHORTCUTVIEW_GROUP = "SHORTCUTVIEW";

using namespace utils::common;

QString ss(const QString &text)
{
    QString str = dApp->setter->value(SHORTCUTVIEW_GROUP, text).toString();
    str.replace(" ", "");
    return str;
}
}  //namespace

ThumbnailListView::ThumbnailListView(QString imgtype)
    : m_model(new QStandardItemModel(this))
{
    m_imageType = imgtype;

    m_iDefaultWidth = 0;
    m_iBaseHeight = BASE_HEIGHT;
    m_albumMenu = nullptr;

//    setViewportMargins(LEFT_MARGIN, 0, RIGHT_MARGIN, 0);
    setIconSize(QSize(400, 400));
    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);
//    setFlow(QListView::LeftToRight);
    setSpacing(ITEM_SPACING);
    setDragEnabled(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
	setMinimumWidth(800);

    m_delegate = new ThumbnailDelegate();
    m_delegate->m_imageTypeStr = m_imageType;

    setItemDelegate(m_delegate);
    setModel(m_model);

    m_pMenu = new DMenu();

    initMenuAction();
    initConnections();
}

ThumbnailListView::~ThumbnailListView()
{

}

void ThumbnailListView::initConnections()
{
    connect(this, &QListView::customContextMenuRequested, this, &ThumbnailListView::onShowMenu);
    connect(m_pMenu, &DMenu::triggered, this, &ThumbnailListView::onMenuItemClicked);
	connect(this,&ThumbnailListView::doubleClicked,this,[=](const QModelIndex &index){
        qDebug()<<"index is "<<index.row();
        if (m_imageType.compare(COMMON_STR_TRASH) != 0) {
            emit openImage(index.row());
        }
    });
    connect(this,&ThumbnailListView::clicked,this,[=](){
            emit hideExtensionPanel();
    });
    connect(dApp->signalM, &SignalManager::sigMainwindowSliderValueChg, this, &ThumbnailListView::onPixMapScale);
    connect(m_delegate, &ThumbnailDelegate::sigCancelFavorite, this, &ThumbnailListView::onCancelFavorite);
}

void ThumbnailListView::calBasePixMapWandH()
{
    for(int i = 0; i < m_ItemList.length(); i++)
    {
        if (0 == m_ItemList[i].height)
        {
            m_ItemList[i].width = m_iBaseHeight;
        }
        else
        {
            m_ItemList[i].width = m_ItemList[i].width * m_iBaseHeight / m_ItemList[i].height;
        }

        m_ItemList[i].height = m_iBaseHeight;
    }
}

void ThumbnailListView::calWidgetItemWandH()
{
    int i_baseWidth = 0;
    int i_totalwidth = width() - 36;

    QList<int> rowWidthList;
    QList<ItemInfo> itemInfoList;

    m_gridItem.clear();

    for(int i = 0; i< m_ItemList.length(); i++)
    {
        if ((i_baseWidth + m_ItemList[i].width) <= i_totalwidth)
        {
            i_baseWidth = i_baseWidth + m_ItemList[i].width + ITEM_SPACING;
            itemInfoList<<m_ItemList[i];

            if (i == m_ItemList.length() -1 )
            {
                i_baseWidth -= ITEM_SPACING;
                rowWidthList<<i_baseWidth;
                m_gridItem<<itemInfoList;
            }
        }
        else
        {
            i_baseWidth -= ITEM_SPACING;
            rowWidthList<<i_baseWidth;
            i_baseWidth = m_ItemList[i].width + ITEM_SPACING;

            m_gridItem<<itemInfoList;
            itemInfoList.clear();
            itemInfoList<<m_ItemList[i];

            if (i == m_ItemList.length() -1 )
            {
                i_baseWidth -= ITEM_SPACING;
                rowWidthList<<i_baseWidth;
                m_gridItem<<itemInfoList;
            }
        }
    }

    for(int i = 0; i < rowWidthList.length(); i++)
    {
        if (i == rowWidthList.length() - 1)
        {
            break;
        }

        int rowWidth = 0;
        for(int j = 0; j < m_gridItem[i].length(); j++)
        {
            m_gridItem[i][j].width = m_gridItem[i][j].width * i_totalwidth / rowWidthList[i];
            m_gridItem[i][j].height = m_gridItem[i][j].height * i_totalwidth / rowWidthList[i];

            rowWidth = rowWidth + m_gridItem[i][j].width + ITEM_SPACING;
        }

        rowWidthList[i] = rowWidth - ITEM_SPACING;
    }


    if (1 < rowWidthList.length() && rowWidthList[0] < i_totalwidth)
    {
        m_gridItem[0][0].width = m_gridItem[0][0].width + i_totalwidth - rowWidthList[0];
    }

    if (0 < m_gridItem.length())
    {
        m_height = 0;
        for(int i = 0; i < rowWidthList.length(); i++)
        {
            m_height = m_height + m_gridItem[i][0].height;
        }
    }
}

void ThumbnailListView::addThumbnailView()
{
    m_model->clear();
    for(int i = 0; i < m_gridItem.length(); i++)
    {
        for(int j = 0; j < m_gridItem[i].length(); j++)
        {
            QStandardItem *item = new QStandardItem;

            QVariantList datas;
            datas.append(QVariant(m_gridItem[i][j].name));
            datas.append(QVariant(m_gridItem[i][j].path));
            datas.append(QVariant(m_gridItem[i][j].width));
            datas.append(QVariant(m_gridItem[i][j].height));
            datas.append(QVariant(m_gridItem[i][j].remainDays));
            datas.append(QVariant(m_gridItem[i][j].image));

            item->setData(QVariant(datas), Qt::DisplayRole);
            item->setData(QVariant(QSize(m_gridItem[i][j].width, m_gridItem[i][j].height)), Qt::SizeHintRole);
            m_model->appendRow(item);
        }
    }
}

void ThumbnailListView::updateThumbnailView()
{
    int index = 0;
    for(int i = 0; i < m_gridItem.length(); i++)
    {
        for(int j = 0; j < m_gridItem[i].length(); j++)
        {
            QSize picSize(m_gridItem[i][j].width, m_gridItem[i][j].height);

            m_model->item(index, 0)->setSizeHint(picSize);
            index++;
        }
    }
}

void ThumbnailListView::insertThumbnails(const QList<ItemInfo> &itemList)
{
    m_ItemList = itemList;

    for(int i = 0; i < m_ItemList.length(); i++)
    {
        QImage tImg;

        m_ItemList[i].width = m_ItemList[i].image.width();
        m_ItemList[i].height = m_ItemList[i].image.height();
    }

    calBasePixMapWandH();

    if (0 != m_iDefaultWidth)
    {
        calWidgetItemWandH();
        addThumbnailView();
    }
}

void ThumbnailListView::onShowMenu(const QPoint &pos)
{
    //外接设备显示图片时，禁用鼠标右键菜单
    if (!this->indexAt(pos).isValid() || ALBUM_PATHTYPE_BY_PHONE == m_imageType)
    {
        return;
    }

    updateMenuContents();
    m_pMenu->popup(QCursor::pos());
}

void ThumbnailListView::updateMenuContents()
{
    if (m_imageType.compare(COMMON_STR_TRASH) == 0) {
        return;
    }

    QStringList paths = selectedPaths();
    paths.removeAll(QString(""));

    foreach (QAction* action , m_MenuActionMap.values()) {
        action->setVisible(true);
    }

    if (1 != paths.length())
    {
        m_MenuActionMap.value(VIEW_CONTEXT_MENU)->setVisible(false);
        m_MenuActionMap.value(FULLSCREEN_CONTEXT_MENU)->setVisible(false);
    }
    if (COMMON_STR_TRASH == m_imageType)
    {
        m_MenuActionMap.value(THROWTOTRASH_CONTEXT_MENU)->setVisible(false);
    }
    else
    {
        m_albumMenu->deleteLater();
        m_albumMenu = createAlbumMenu();
        if (m_albumMenu) {
            QAction * action = m_MenuActionMap.value(EXPORT_CONTEXT_MENU);
            m_pMenu->insertMenu(action, m_albumMenu);
        }
    }


    if (1 == paths.length() && COMMON_STR_TRASH != m_imageType)
    {
        if (DBManager::instance()->isImgExistInAlbum(COMMON_STR_FAVORITES, paths[0]))
        {
            m_MenuActionMap.value(FAVORITE_CONTEXT_MENU)->setVisible(false);
        }
        else
        {
            m_MenuActionMap.value(UNFAVORITE_CONTEXT_MENU)->setVisible(false);
        }

        m_pMenu->addSeparator();
    }

    int flag_imageSupportSave = 0;
    for(auto path: paths)
    {
        if(!utils::image::imageSupportSave(path))
        {
            flag_imageSupportSave = 1;
            break;
        }
    }

    if(0 == flag_imageSupportSave)
    {
        int flag_isRW = 0;
        for(auto path: paths){
            if (QFileInfo(path).isReadable() && !QFileInfo(path).isWritable())
            {
                flag_isRW = 1;
                break;
            }
        }

        if(flag_isRW == 1)
        {
            m_MenuActionMap.value(ROTATECLOCKWISE_CONTEXT_MENU)->setDisabled(true);
            m_MenuActionMap.value(ROTATECOUNTERCLOCKWISE_CONTEXT_MENU)->setDisabled(true);
        }
        else
        {
            m_MenuActionMap.value(ROTATECLOCKWISE_CONTEXT_MENU)->setDisabled(false);
            m_MenuActionMap.value(ROTATECOUNTERCLOCKWISE_CONTEXT_MENU)->setDisabled(false);
        }
    }
    else
    {
        m_MenuActionMap.value(ROTATECLOCKWISE_CONTEXT_MENU)->setVisible(false);
        m_MenuActionMap.value(ROTATECOUNTERCLOCKWISE_CONTEXT_MENU)->setVisible(false);
    }

    if (1 != paths.length()) {
        m_MenuActionMap.value(DISPLAYINFILEMANAGER_CONTEXT_MENU)->setVisible(false);
        m_MenuActionMap.value(ImageInfo_CONTEXT_MENU)->setVisible(false);
    }
    if (!(1 == paths.length() && utils::image::imageSupportSave(paths[0]))) {
        m_MenuActionMap.value(SETASWALLPAPER_CONTEXT_MENU)->setVisible(false);
    }
}

void ThumbnailListView::appendAction(int id, const QString &text, const QString &shortcut)
{
    QAction *ac = new QAction();
    addAction(ac);
    ac->setText(text);
    ac->setProperty("MenuID", id);
    //如果是查看图片，需要响应Enter键，而Enter键有两个Key-Enter和Return
    if (text.compare(VIEW_CONTEXT_MENU) == 0) {
        QList<QKeySequence> shortcuts;
        shortcuts.append(QKeySequence(ENTER_SHORTCUT));
        shortcuts.append(QKeySequence(RETURN_SHORTCUT));
        ac->setShortcuts(shortcuts);
    } else {
        ac->setShortcut(QKeySequence(shortcut));
    }
    ac->setShortcutContext(Qt::WidgetShortcut);
    m_MenuActionMap.insert(text, ac);
    m_pMenu->addAction(ac);
}

void ThumbnailListView::initMenuAction()
{
    m_pMenu->clear();
    if (m_imageType.compare(COMMON_STR_TRASH) == 0) {
        appendAction(IdImageInfo, tr(ImageInfo_CONTEXT_MENU), ss(ImageInfo_CONTEXT_MENU));
        appendAction(IdMoveToTrash, tr(DELETE_CONTEXT_MENU), ss(THROWTOTRASH_CONTEXT_MENU));
        appendAction(IdTrashRecovery, tr(BUTTON_RECOVERY), ss(BUTTON_RECOVERY));
        return;
    }

    m_MenuActionMap.clear();
    appendAction(IdView, tr(VIEW_CONTEXT_MENU), ss(VIEW_CONTEXT_MENU));
    appendAction(IdFullScreen, tr(FULLSCREEN_CONTEXT_MENU), ss(FULLSCREEN_CONTEXT_MENU));
    appendAction(IdStartSlideShow, tr(SLIDESHOW_CONTEXT_MENU), ss(SLIDESHOW_CONTEXT_MENU));

    m_pMenu->addSeparator();
    appendAction(IdExport, tr(EXPORT_CONTEXT_MENU), ss(EXPORT_CONTEXT_MENU));
    appendAction(IdCopyToClipboard, tr(COPYTOCLIPBOARD_CONTEXT_MENU), ss(COPYTOCLIPBOARD_CONTEXT_MENU));
    appendAction(IdMoveToTrash, tr(DELETE_CONTEXT_MENU), ss(THROWTOTRASH_CONTEXT_MENU));
    m_pMenu->addSeparator();
    appendAction(IdRemoveFromFavorites, tr(UNFAVORITE_CONTEXT_MENU), ss(UNFAVORITE_CONTEXT_MENU));
    appendAction(IdAddToFavorites, tr(FAVORITE_CONTEXT_MENU), ss(FAVORITE_CONTEXT_MENU));
    m_pMenu->addSeparator();
    appendAction(IdRotateClockwise, tr(ROTATECLOCKWISE_CONTEXT_MENU),
                 ss(ROTATECLOCKWISE_CONTEXT_MENU));
    appendAction(IdRotateCounterclockwise, tr(ROTATECOUNTERCLOCKWISE_CONTEXT_MENU),
                 ss(ROTATECOUNTERCLOCKWISE_CONTEXT_MENU));
    m_pMenu->addSeparator();
    appendAction(IdSetAsWallpaper, tr(SETASWALLPAPER_CONTEXT_MENU), ss(SETASWALLPAPER_CONTEXT_MENU));
    appendAction(IdDisplayInFileManager, tr(DISPLAYINFILEMANAGER_CONTEXT_MENU),
                 ss(DISPLAYINFILEMANAGER_CONTEXT_MENU));
    appendAction(IdImageInfo, tr(ImageInfo_CONTEXT_MENU), ss(ImageInfo_CONTEXT_MENU));
}

QMenu *ThumbnailListView::createAlbumMenu()
{
    QMenu *am = new QMenu(tr("添加到相册"));

    QStringList albums = DBManager::instance()->getAllAlbumNames();
    albums.removeAll(COMMON_STR_FAVORITES);
    albums.removeAll(COMMON_STR_TRASH);
    albums.removeAll(COMMON_STR_RECENT_IMPORTED);

    QAction *ac = new QAction(am);
    ac->setProperty("MenuID", IdAddToAlbum);
    ac->setText(tr("新建相册"));
    ac->setData(QString("Add to new album"));
    am->addAction(ac);
    am->addSeparator();
    for (QString album : albums)
    {
        QAction *ac = new QAction(am);
        ac->setProperty("MenuID", IdAddToAlbum);
        ac->setText(fontMetrics().elidedText(QString(album).replace("&", "&&"), Qt::ElideMiddle, 200));
        ac->setData(album);
        am->addAction(ac);
    }

    return am;
}

void ThumbnailListView::onMenuItemClicked(QAction *action)
{
    QStringList paths = selectedPaths();
    paths.removeAll(QString(""));
    if (paths.isEmpty()) {
        return;
    }
//    const QStringList viewPaths = (paths.length() == 1) ? albumPaths() : paths;
    const QString path = paths.first();
    const int id = action->property("MenuID").toInt();
    switch (MenuItemId(id)) {
    case IdView:
        emit menuOpenImage(path, paths, false, false);
        break;
    case IdFullScreen:
        emit menuOpenImage(path, paths, true, false);
        break;
    case IdStartSlideShow:
        emit menuOpenImage(path, paths, true, true);
        break;
    case IdAddToAlbum: {
        const QString album = action->data().toString();
        if (album != "Add to new album") {
           DBManager::instance()->insertIntoAlbum(album, paths);
        }
        else {
            emit dApp->signalM->createAlbum(paths);
        }
        break;
    }
    case IdCopyToClipboard:
        utils::base::copyImageToClipboard(paths);
        break;
    case IdMoveToTrash:
    {
        if (IMAGE_DEFAULTTYPE != m_imageType
            && COMMON_STR_RECENT_IMPORTED != m_imageType
            && COMMON_STR_TRASH != m_imageType
                && COMMON_STR_FAVORITES != m_imageType)
        {
            DBManager::instance()->removeFromAlbum(m_imageType, paths);
        }

        else if (COMMON_STR_TRASH == m_imageType)
        {
            ImgDeleteDialog *dialog = new ImgDeleteDialog(paths.length());
            dialog->show();
            connect(dialog,&ImgDeleteDialog::imgdelete,this,[=]{
                for(auto path : paths)
                {
                    dApp->m_imagetrashmap.remove(path);
                }

                DBManager::instance()->removeTrashImgInfos(paths);
            });
        }
        else
        {
            DBImgInfoList infos;
            for(auto path : paths)
            {
                DBImgInfo info;
                info = DBManager::instance()->getInfoByPath(path);
                info.time = QDateTime::currentDateTime();
                infos<<info;

                dApp->m_imagemap.remove(path);
            }

            dApp->m_imageloader->addTrashImageLoader(paths);
            DBManager::instance()->insertTrashImgInfos(infos);
            DBManager::instance()->removeImgInfos(paths);
        }
    }
        break;
    case IdAddToFavorites:
        DBManager::instance()->insertIntoAlbum(COMMON_STR_FAVORITES, paths);
        break;
    case IdRemoveFromFavorites:
        DBManager::instance()->removeFromAlbum(COMMON_STR_FAVORITES, paths);
        break;
    case IdRemoveFromAlbum:
    {
        ImgDeleteDialog *dialog = new ImgDeleteDialog(paths.length());
        dialog->show();
        connect(dialog,&ImgDeleteDialog::imgdelete,this,[=]{
            DBManager::instance()->removeFromAlbum(m_imageType, paths);
        });
    }
        break;
    case IdRotateClockwise:
    {
        for(QString path : paths)
        {
            utils::image::rotate(path, 90);
        }

        if (COMMON_STR_TRASH == m_imageType)
        {
            dApp->m_imageloader->updateTrashImageLoader(paths);
        }
        else
        {
            dApp->m_imageloader->updateImageLoader(paths);
        }
    }
        break;
    case IdRotateCounterclockwise:
    {
        for(QString path : paths)
        {
            utils::image::rotate(path, -90);
        }

        if (COMMON_STR_TRASH == m_imageType)
        {
            dApp->m_imageloader->updateTrashImageLoader(paths);
        }
        else
        {
            dApp->m_imageloader->updateImageLoader(paths);
        }
    }
        break;
    case IdSetAsWallpaper:
        dApp->wpSetter->setWallpaper(path);
        break;
    case IdDisplayInFileManager:
        utils::base::showInFileManager(path);
        break;
    case IdImageInfo:
        emit dApp->signalM->showImageInfo(path);
        break;
    case IdExport:
        emit dApp->signalM->exportImage(paths);
        break;
    case IdTrashRecovery:
        emit trashRecovery();
        break;
    default:
        break;
    }

//    updateMenuContents();
}

QStringList ThumbnailListView::selectedPaths()
{
    QStringList paths;
    for (QModelIndex index : selectionModel()->selectedIndexes()) {
        const QVariantList datas =
                index.model()->data(index, Qt::DisplayRole).toList();
        if (datas.length() == 6) {
            paths << datas[1].toString();
        }
    }

    return paths;
}

QList<ThumbnailListView::ItemInfo> ThumbnailListView::getAllPaths()
{
    return m_ItemList;
}

void ThumbnailListView::onPixMapScale(int value)
{
    switch(value)
    {
    case 0:
        m_iBaseHeight = 80;
        break;
    case 1:
        m_iBaseHeight = 90;
        break;
    case 2:
        m_iBaseHeight = 100;
        break;
    case 3:
        m_iBaseHeight = 110;
        break;
    case 4:
        m_iBaseHeight = 120;
        break;
    }

    calBasePixMapWandH();
    calWidgetItemWandH();
    addThumbnailView();
    emit loadend(m_height+15);
}

void ThumbnailListView::onCancelFavorite(const QModelIndex &index)
{
    QStringList str;
    QVariantList datas = index.model()->data(index, Qt::DisplayRole).toList();

    if (datas.length() >= 2) {
        str<<datas[1].toString();
    }

    DBManager::instance()->removeFromAlbumNoSignal(COMMON_STR_FAVORITES, str);

    m_model->removeRow(index.row());
}



void ThumbnailListView::resizeEvent(QResizeEvent *e)
{
    if (0 == m_iDefaultWidth)
    {
        calWidgetItemWandH();
        addThumbnailView();
    }
    else
    {
        calWidgetItemWandH();
        updateThumbnailView();
    }
//    emit loadend((m_height)*m_gridItem.size()+15);
    emit loadend(m_height+15);

    m_iDefaultWidth = width();

    QListView::resizeEvent(e);
}

void ThumbnailListView::mouseMoveEvent(QMouseEvent *event)
{
    if (COMMON_STR_TRASH == m_imageType)
    {
        emit dApp->signalM->sigBoxToChoose();
    }

    QAbstractItemView::mouseMoveEvent(event);
}

void ThumbnailListView::mousePressEvent(QMouseEvent *event)
{
    if (COMMON_STR_TRASH == m_imageType)
    {
        if (!this->indexAt(event->pos()).isValid())
        {
            emit dApp->signalM->sigTrashViewBlankArea();

        }
    }

    QAbstractItemView::mousePressEvent(event);

    if (!this->indexAt(event->pos()).isValid())
    {
        emit sigTimeLineItemBlankArea();
    }
}