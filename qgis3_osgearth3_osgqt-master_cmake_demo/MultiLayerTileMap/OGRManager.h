//
// Created by TictorDC on 2022/7/6.
//

#ifndef TILEMAPMANAGERDEMO_OGRMANAGER_H
#define TILEMAPMANAGERDEMO_OGRMANAGER_H

#include "ogrsf_frmts.h"

namespace MultiLayerTileMap {
    class OGRManager {
    private:
        std::map<std::string, GDALDataset *> ogrDataPool;
    public:
        OGRManager() {
            OGRRegisterAll(); // 注册所有需要的格式驱动程序
            ogrDataPool = std::map<std::string, GDALDataset *>(); // 初始化数据池
        }

        ~OGRManager() {
            for (auto &it: ogrDataPool) {
                GDALClose(it.second); // 关闭数据集
            }
        }

        /// 添加数据集
        /// \param filename 文件路径
        /// \param datasetName 数据集名称，不能与已有数据集冲突
        /// \return nullptr 为添加失败
        GDALDataset *loadDataset(const std::string &filename, const std::string &datasetName);

        /// 根据数据集的名字查找数据集
        /// \param datasetName loadDataset 所设置的名字
        /// \return nullptr 为查找失败
        GDALDataset *getDatasetByName(const std::string &datasetName);

        /// 根据坐标，给出第 layerIndex 层中对应的若干个 feature 的第 feildIndex 个属性
        /// 具体的属性可以在 qgis/postgis 等中提前查看
        /// \param layerIndex 查找属性的对应层
        /// \param filedIndex 属性的列号
        /// \param lon 经度
        /// \param lat 维度
        /// \return
        std::vector<std::string>
        getFieldByLonLat(const std::string &datasetName, int layerIndex, int filedIndex, double lon, double lat);
    };
}

#endif //TILEMAPMANAGERDEMO_OGRMANAGER_H
