#include "stdafx.h"

#pragma execution_character_set("utf-8")

#include "ZtViewer.h"
#include "ztLayout.h"
#include "ztThread.h"
#include "ztTable.h"
#include "ztReadJson.h"

#include "LiDataHandlers/ztCompositeViewer.h"
#include "LiDataHandlers/ztLiDataHandler.h"
#include "LiDataHandlers/ztViewHandler.h"
#include "LiDataHandlers/ztColorHandler.h"
#include "LiDataHandlers/ztMeasureHandler.h"
#include "LiDataHandlers/ztSectionHandler.h"
#include "LiDataHandlers/ztObjectHandler.h"
#include "LiDataHandlers/ztClipboxHandler.h"
#include "LiDataHandlers/ztBTPHandler.h"
#include "liDataHandlers/ztUtil.h"


#include "ztMeasureDialog.h"
#include "ztClipboxDialog.h"

namespace zt{

	ZtViewer::ZtViewer(QMainWindow* mainWnd)
	{
		// 建立布局
		_layout = new ztLayout;
		_layout->createLayout(mainWnd);
		this->createLayoutConnetion();
		VIEWER->getChildView()->addEventHandler(VIEWHANDLER);
		VIEWER->getChildView()->addEventHandler(DATAHANDLER);
		VIEWER->getChildView()->addEventHandler(COLORHANDLER);
		VIEWER->getChildView()->addEventHandler(MEASUREHANDLER);
		VIEWER->getChildView()->addEventHandler(SECTIONHANDLER);
		VIEWER->getChildView()->addEventHandler(OBJECTHANDLER);
		VIEWER->getChildView()->addEventHandler(CLIPHANDLER);
		VIEWER->getChildView()->addEventHandler(BTPHANDLER);
		_layout->cloudClassify();
		connect(_layout->gettablewidget(), SIGNAL(cellChanged(int, int)), this, SLOT(cloudStatus(int, int)));
		connect(_layout->gettablewidget(), SIGNAL(cellDoubleClicked(int, int)), this, SLOT(colorChange(int, int)));

	}

	QWidget* ZtViewer::getViewer()
	{
		return VIEWER;
	}

	void ZtViewer::createLayoutConnetion()
	{
		connect(_layout, SIGNAL(showNameClouds(QString)), this, SLOT(showPointClouds(QString)));
		connect(_layout, SIGNAL(hideNameClouds(QString)), this, SLOT(hidePointClouds(QString)));
		connect(_layout, SIGNAL(deleteNameClouds(QString)), this, SLOT(deletePointClouds(QString)));
		connect(DATAHANDLER, SIGNAL(progressChanged(int)), _layout, SLOT(setProgressBar(int)));
	}

	void ZtViewer::loadData()
	{
		QStringList path_list = QFileDialog::getOpenFileNames(this, tr("Import Data"),
			/*tr("E:\\")*/NULL, "TIF(*.tif);;LAS(*.las);;XYZ(*.xyz);;PCD(*.pcd);;PLY(*.ply);;OSG(*.osg)");

		//QStringList path_list;
		//path_list.push_back("E:\\JSJX-1-5.tif");
		if (!path_list.empty())
		{
			// 先统一开启渐进读取
			DATAHANDLER->startMapFile(path_list);

			VIEWER->getChildView()->home();
			_layout->showCloudsTree(path_list);

		}
	}

	void ZtViewer::mapData()
	{
		QSettings setting("ztLidar", "App");
		QString lastPath = setting.value("filePath").toString();

		QStringList path_list;
		//path_list.push_back("E:\\JSJX-1-5.las");
		//path_list.push_back("E:\\JSJX-1-5.tif");
		//path_list.push_back("E:\\Data\\Kunming\\tile0002.tif");

		path_list.push_back("D:\\code\\suidao\\LAS2.las");
		//path_list.push_back("E:\\Data\\Suidao\\LAS3.las");
		path_list.push_back("D:\\code\\suidao\\test.btp");

		//path_list.push_back("E:\\Data\\Suidao\\LAS2.las");
		//path_list.push_back("E:\\Data\\Suidao\\LAS3.las");
		//path_list.push_back("E:\\Data\\Suidao\\test.btp");


		path_list = QFileDialog::getOpenFileNames(this, tr("Import Data"),
			/*tr("E:\\")*/lastPath, "LAS(*.las);;TIF(*.tif);;BTP(*.btp);;XYZ(*.xyz);;PCD(*.pcd);;PLY(*.ply);;OSG(*.osg)");

		if (!path_list.empty())
		{
			setting.setValue("filePath", path_list[0]);

			// 先读取基础点云
			DATAHANDLER->startMapFile(path_list);
			VIEWHANDLER->originHomePosition(VIEWER->getChildView());

			// 开启渐进加载
			//DATAHANDLER->lockAllFiles();

			DATAHANDLER->unlockMaps();
			DATAHANDLER->runAllMapsInSingle(VIEWER->getChildView());

			_layout->showCloudsTree(path_list);

		}
	}

	void ZtViewer::paintData()
	{
		readJson();

		COLORHANDLER->setColorType(3);
		DATAHANDLER->cleanAllClouds();
		DATAHANDLER->runAllMapsInSingle(VIEWER->getChildView());
	}

	void ZtViewer::saveData()
	{
		QStringList path_list = QFileDialog::getOpenFileNames(this, tr("Resample Data"),
			/*tr("E:\\")*/NULL, "LAS(*.las)");

		for (int i = 0; i < path_list.size();i++)
		{
			DATAHANDLER->addData(ztUtil::qstr2str(path_list[i]), false);
		}
		DATAHANDLER->resampleAllFiles();

	}

	void ZtViewer::showPointClouds(QString name)
	{
		DATAHANDLER->showClouds(QStringList(name));
	}

	void ZtViewer::hidePointClouds(QString name)
	{
		DATAHANDLER->hideClouds(QStringList(name));
	}

	void ZtViewer::buildFileTree()
	{
		QStringList file_list = DATAHANDLER->getCloudFileNameList();
		_layout->showCloudsTree(file_list);
	}

	void ZtViewer::loadTable()
	{
		QStringList path_list = QFileDialog::getOpenFileNames(this, tr("Import Table"),
			/*tr("E:\\")*/NULL, "CSV(*.csv)");

		if (path_list.empty()) return;
		else
		{
			ztTable* table = new ztTable;
			table->readCSV(path_list);
			_layout->tableDislayCsv(table->m_table);
		}
	}

	void ZtViewer::measureData()
	{
		MEASUREHANDLER->beginMeasure();
		ztMeasureDialog* dialog = new ztMeasureDialog;
		connect(MEASUREHANDLER, SIGNAL(displayPoint(double, double, double)), 
			dialog, SLOT(displayPoint(double, double, double)));

		connect(MEASUREHANDLER, SIGNAL(closeMeasureDialog()), dialog, SLOT(hide()));

		dialog->setWindowFlags(  // 对话框前置
			dialog->windowFlags() | Qt::WindowStaysOnTopHint);
		dialog->setGeometry(200, 200, dialog->width(), dialog->height());
		dialog->show();
	}

	void ZtViewer::cloudStatus(int row, int column)
	{
		if (column == 0)
		{
			std::vector<int>cloud_list;
			std::vector<int>::iterator iter;
			QString cloud_class;
			COLORHANDLER->getClassIds(cloud_list);
			QStringList cloudclass_list = _layout->getcloudClass();
			cloud_class = _layout->gettablewidget()->item(row, 2)->text();
			if (_layout->gettablewidget()->item(row, column)->checkState() == Qt::Unchecked)
			{

				for (iter = cloud_list.begin(); iter != cloud_list.end();)
				{
					if (*iter == row )
					{
						iter = cloud_list.erase(iter);
						break;
					}
					else
						iter++;
				}

				COLORHANDLER->setClassIds(cloud_list);
				COLORHANDLER->refreshColor();

			}
			else
			{
				cloud_list.push_back(row );
				COLORHANDLER->setClassIds(cloud_list);
				COLORHANDLER->refreshColor();

			}
		}
	}

	void ZtViewer::saveJson()
	{
		QTableWidget* tableWidegt=_layout->gettablewidget();
		int column = tableWidegt->columnCount();
		int row = tableWidegt->rowCount();
		cJSON* root= cJSON_CreateObject();

		for (int i = 0; i < row; ++i)
		{
			QColor color = tableWidegt->item(i, 1)->backgroundColor();
			QString cloudClassName = tableWidegt->item(i, 2)->text();
			cJSON* next = cJSON_CreateObject();
			double Color[3];
			Color[0] = color.red();
			Color[1] = color.green();
			Color[2] = color.blue();
			std::stringstream ss;
			ss << "Cloud_class" << i+1;
			std::string cloudClass = ztUtil::qstr2str(cloudClassName);
			cJSON_AddItemToObject(next, "CloudClassId", cJSON_CreateNumber(i+1));
			cJSON_AddItemToObject(next, "CloudName", cJSON_CreateString(cloudClass.c_str()));
			cJSON_AddItemToObject(next, "CloudColor", cJSON_CreateDoubleArray(Color, 3));
			cJSON_AddItemToObject(root, ss.str().c_str(), next);
		}
		cJSON_AddItemToObject(root, "CloudClassNumber", cJSON_CreateNumber(row));
		FILE* fp = fopen("PointCloudClass.json", "w");
		char* buff = NULL;
		buff = cJSON_Print(root);
		fwrite(buff, strlen(buff), 1, fp);
		fclose(fp);

		QMessageBox::information(this, "提示", "文件已生成");
	}

	void ZtViewer::readJson()
	{
		QStringList path_list = QFileDialog::getOpenFileNames(this, tr("Import Data"),
			/*tr("E:\\")*/NULL, "Json(*.json)");
		if (!path_list.empty())
		{
			ztReadJson * read_json =  ztReadJson::getHandler();
			read_json->readJson(path_list[0]);
			_layout->repaintCloudClass();
			std::vector<cloudClassStruct> cloudInformation;
			read_json->getCloudClassInformation(cloudInformation);
			std::vector<osg::Vec4> color4;
			COLORHANDLER->getClassifyColors(color4);
			for (int i = 0; i < 16; ++i)
			{
				color4[i] = osg::Vec4f(cloudInformation[i].color.redF(), 
					cloudInformation[i].color.greenF(), cloudInformation[i].color.blueF(), 1.0);
			}
			COLORHANDLER->setClassifyColors(color4);
			COLORHANDLER->refreshColor();
		}

	}

	void ZtViewer::colorChange(int row, int column)
	{
		if (column == 1)
		{
			QColor color = QColorDialog::getColor();
			_layout->gettablewidget()->item(row, column)->setBackgroundColor(color);
			std::vector<osg::Vec4> color4;
			COLORHANDLER->getClassifyColors(color4);
			float r, g, b;
			r = color.redF();
			g = color.greenF();
			b = color.blueF();
			color4[row] = osg::Vec4f(r,g,b,1.0);
			COLORHANDLER->setClassifyColors(color4);
			COLORHANDLER->refreshColor();

		}
	}

	void ZtViewer::openDxf()
	{
		QStringList path_list = QFileDialog::getOpenFileNames(this, tr("Import Data"),
			/*tr("E:\\")*/NULL, "Dxf(*.dxf)");

		if (path_list.empty()) return;
		else
		{
			_layout->openDxf(path_list[0]);
		}
	}

	void ZtViewer::writeDxf()
	{
		QString filename = "test.dxf";
		_layout->writeDxf(filename);
	}
	void ZtViewer::viewData()
	{
		//CLIPHANDLER->begin2Clip();
		ztClipboxDialog* dialog = new ztClipboxDialog;

		dialog->setWindowFlags(  // 对话框前置
			dialog->windowFlags() | Qt::WindowStaysOnTopHint);
		dialog->setGeometry(600, 200, dialog->width(), dialog->height());
		dialog->show();
	}

	void ZtViewer::deletePointClouds(QString name)
	{
		QStringList file_path = DATAHANDLER->getCloudFileNameList();
		// 将传入的点云文件名与LIDATA中的文件名列表比对
		//for (int i = 0; i < file_name.size(); i++){
			for (int j = 0; j < file_path.size(); j++)
			{
				QFileInfo file_info(file_path[j]);
				//if (file_info.fileName() == file_name[i]){
				if (file_info.fileName() == name){
					DATAHANDLER->cleanMapFile(
						ztUtil::qstr2str(file_path[j]), false);
				}
				else continue;
			}
		//}
	}

	void ZtViewer::readShp()
	{
		QStringList path_list = QFileDialog::getOpenFileNames(this, tr("Import Data"),
			NULL, "Shp(*.shp)");

		if (path_list.empty()) return;
		else
		{
			_layout->readShp(path_list[0]);
		}
	}

}
