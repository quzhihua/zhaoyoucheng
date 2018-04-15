#pragma once

#include "ztviewer_global.h"

class QMainWindow;

namespace zt{
	class ztLayout;

	class ZTVIEWER_EXPORT ZtViewer : public QWidget
	{
		Q_OBJECT

	public:
		ZtViewer(QMainWindow* mainWnd);
		~ZtViewer(){};

		// 获取基本osgViewer类
		QWidget* getViewer();
		
		// 创建基本布局消息
		void createLayoutConnetion();

		// 加载数据(打开数据加载面板）
		void loadData();

		// 加载表格(打开表格加载面板）
		void loadTable();

		// 渐进加载（打开渐进加载面板）
		void mapData();

		// 存储数据（打开存储面板）
		void saveData();

		//// 量测数据（打开量测数据面板）
		void measureData();

		//读取json文件
		void readJson();

		//获得点云分类信息并存储为json文件
		void saveJson();

		//读取shp文件
		void readShp();

		// 浏览数据（打开浏览数据面板）
		void viewData();

		//// 检校数据（打开检校数据面板）
		//virtual void ajustData() = 0;

		//// 渲染数据（打开数据渲染面板）
		void paintData();

		public slots:

		void buildFileTree();

		//显示特定点云
		void showPointClouds(QString name);

		//隐藏特定点云
		void hidePointClouds(QString name);

		//删除特定点云
		void deletePointClouds(QString name);

		//某类点云状态改变
		void cloudStatus(int, int);

		//改变点云颜色
		void colorChange(int, int);

		//����dxf
		void openDxf();
		//����dxf�ļ�
		void writeDxf();


	protected:

	private:
		ZtViewer();
		ztLayout* _layout;
		static ZtViewer* _viewer;

	};

}
