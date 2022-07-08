//
// Created by TictorDC on 2022/7/6.
//

#include "OGRManager.h"
#include <iostream>

using namespace std;
namespace MultiLayerTileMap {
    GDALDataset *OGRManager::loadDataset(const string &filename, const string &datasetName) {
        auto *dataset = static_cast<GDALDataset *>(
                GDALOpenEx(filename.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
        if (dataset == nullptr) {
            cerr << "数据集加载失败" << endl;
            return nullptr;
        }
        if (ogrDataPool.find(filename) != ogrDataPool.end()) {
            cerr << "已有同名数据集" << endl;
            return nullptr;
        }
        auto a = dataset->GetLayer(0)->GetSpatialRef();
        cout << a->GetName() << endl;
        ogrDataPool.insert(make_pair(datasetName, dataset));
        return dataset;
    }

    std::vector<std::string>
    OGRManager::getFieldByLonLat(const string &datasetName, int layerIndex, int filedIndex, double lon, double lat) {
        auto ret = std::vector<std::string>();
        if (ogrDataPool.find(datasetName) == ogrDataPool.end()) {
            cerr << "没有指定的数据集" << endl;
            return ret;
        }
        auto dataset = ogrDataPool[datasetName];
        auto layer = dataset->GetLayer(layerIndex);
        if (layer == nullptr) {
            cerr << "没有指定的数据层" << endl;
            return ret;
        }
        auto point = new OGRPoint(lon, lat);
        auto srs = new OGRSpatialReference;
        srs->SetWellKnownGeogCS("WGS84");
        point->assignSpatialReference(srs);
        point->transformTo(layer->GetSpatialRef());
        layer->SetSpatialFilter(point);
        while (auto it = layer->GetNextFeature()) {
            ret.emplace_back(it->GetFieldAsString(filedIndex));
        }
        return ret;
    }

    GDALDataset *OGRManager::getDatasetByName(const string &datasetName) {
        auto it = ogrDataPool.find(datasetName);
        if (it == ogrDataPool.end()) {
            cerr << "找不到指定数据集" << endl;
            return nullptr;
        }
        return it->second;
    }
}