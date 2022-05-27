import QtQuick 2.11

Item {

    property var imgPaths
    property var imgCurrentPath
    property int imgCurrentIndex: 0
    property int minHeight: 300
    property int minWidth: 628
    property int albumMinHeight: 300
    property int albumMinWidth: 628

    property int minHideHeight: 428
    property int floatMargin: 60
    property int titleHeight: 50
    property int showBottomY: 80
    property int actionMargin: 9//应用图标距离顶栏

    property int thumbnailViewTitleHieght: 85 // 缩略图视图区域标题显示区域高度
    property int verticalScrollBarWidth: 15 // 垂直滚动条宽度

    property int rightMenuItemHeight: 32//右键菜单item的高度
    property int rightMenuSeparatorHeight: 12//右键菜单分割层的高度
    property bool ctrlPressed: false//记录ctrl键是否按下

    property int currentViewIndex: 0// 0:打开图片界面 1:无图片界面
    property int stackControlCurrent: 0// 0:相册界面 1:看图界面
    property var haveImportedPaths //所有导入的图片

    property int thumbnailSizeLevel: 0 //缩略图缩放等级
    property var statusBarNumText //状态栏显示的总数文本内容

    signal sigWindowStateChange()
    signal sigThumbnailStateChange()

    onHaveImportedPathsChanged: {
        currentViewIndex = 2
    }

    //缩略图类型枚举
    enum ThumbnailType {
        Normal,      //普通模式
        Trash,       //最近删除
        CustomAlbum, //自定义相册
        AutoImport   //自动导入路径
    }

}
