#include <QtGui/qopengl.h>
#include <osg/ArgumentParser>
#include <osgGA/GUIEventHandler>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osgEarth/EarthManipulator>
#include <osgDB/ReadFile>
#include <osg/CoordinateSystemNode>
#include <osgEarth/GLUtils>
#include <osg/Quat>
#include <osg/Vec2d>
#include <osg/io_utils>

#include "osgQOpenGL/osgQOpenGLWidget.h"
#include "MultiLayerTileMap/MapLayerManager.h"
#include "MultiLayerTileMap/OGRManager.h"
#include "MultiLayerTileMap/GroundVehicle.h"

#define CATCH_CONFIG_RUNNER
#define PAUSE ;;; // std::getchar()

#include "catch.hpp"
#include "MultiLayerTileMap/BulletManager.h"

using namespace MultiLayerTileMap;
using namespace osgEarth::Util;
using namespace osgDB;
using namespace MultiLayerTileMap;

const float lonF = 92.9431f;
const float latF = 34.9348f;
const double eps = 0.001;
osg::Group *root = nullptr;
osgViewer::Viewer *viewer = nullptr;
MapLayerManager *mapLayerManager = nullptr;
osgQOpenGLWidget *widget = nullptr;
EarthManipulator *em = nullptr;
GroundVehicle *tank = nullptr;
GroundVehicle *spaceShip = nullptr;
OGRManager *ogrManager = nullptr;
BulletManager *bulletManager = nullptr;
BulletManager::BulletTerrainChangedCallback *bulletTerrainCallback = nullptr;
const osg::Vec3d lonLatAlt{92.9431, 34.9348, 4551.91};
const osg::Vec3d lonLatAlt2{92.9504, 34.9586, 9.80443};

TEST_CASE("MapLayerManager(std::string file)") {
  PAUSE
  const std::string filename = "./data/demo06/simple.earth";
  mapLayerManager = new MapLayerManager(filename, root);
  bulletManager = new BulletManager(mapLayerManager->getMapNode().get());
  REQUIRE(mapLayerManager != nullptr);
}

TEST_CASE("addImageLayer") {
  PAUSE
  em->setViewpoint(Viewpoint("12m-center",
							 lonF, latF, 4552,
							 0, -45, 400.0));
  const std::string filename = "./data/demo06/AGM/AGM_BeiluRiver_Image_Level_16.tif";
  const std::string layerName = "AGM_BeiluRiver_Image";
  REQUIRE(mapLayerManager->addImageLayer(filename, layerName));
  REQUIRE(mapLayerManager->findLayerByName(layerName) != nullptr);
}

// TEST_CASE("addShapefileLayer") {
//   PAUSE
//   const std::string filename = "./data/demo06/AGM/AGM_BeiluRiver.shp";
//   const std::string layerName = "shp-layer";
//   REQUIRE(mapLayerManager->addShapefileLayer(filename, layerName));
//   // REQUIRE(mapLayerManager->addShapefileLayerFromPostGIS(
// 	//   "localhost",
// 	//   "5432",
// 	//   "postgres",
// 	//   "postgres",
// 	//   "postgis_32_sample",
// 	//   "agm_beiluriver",
// 	//   layerName
//   // ));
//   REQUIRE(mapLayerManager->findLayerByName(layerName));
// }

TEST_CASE("Tank") {
  const std::string filename = "./data/demo06/M60.obj";

  const osg::Vec3d picHeadingRoll{0, 0, 0};
  const osg::Vec3d scaleFactor{1e-2, 1e-2, 1e-2};
  const std::string shapefileDatasetName = "tankShapefile";
  tank = new GroundVehicle(mapLayerManager,
						   ogrManager,
						   filename,
						   lonLatAlt2,
						   picHeadingRoll,
						   scaleFactor,
						   shapefileDatasetName);
}

// TEST_CASE("SpaceShip") {
//   const std::string filename = "E:\\Download\\OpenSceneGraph-Data-3.4.0\\OpenSceneGraph-Data\\spaceship.osgt";
//
//   const osg::Vec3d picHeadingRoll{0, 0, -45};
//   const osg::Vec3d scaleFactor{2, 2, 2};
//   const std::string shapefileDatasetName = "spaceShipShapefile";
//   spaceShip = new GroundVehicle(mapLayerManager,
// 						   nullptr,
// 						   filename,
// 						   lonLatAlt,
// 						   picHeadingRoll,
// 						   scaleFactor,
// 						   shapefileDatasetName,
// 						   false);
// }

// TEST_CASE("createTerrainFromOSG") {
//   bulletTerrainCallback = new BulletManager::BulletTerrainChangedCallback(bulletManager);
//   mapLayerManager->getMapNode()->getTerrain()->addTerrainCallback(bulletTerrainCallback);
// }

TEST_CASE("addElevationLayer") {
  PAUSE
  const std::string filename = "./data/demo06/AGM/ASTGTMV003_N34E092_dem.tif";
  const std::string layerName = "AGM_BeiluRiver_Dem";
  REQUIRE(mapLayerManager->addElevationLayer(filename, layerName));
  REQUIRE(mapLayerManager->findLayerByName(layerName) != nullptr);
}

TEST_CASE("createTerrain") {
  PAUSE
  const std::string url = "./data/demo06/AGM/AGM_BeiluRiver.shp";
  OGRFeatureSource* ogrSource = new OGRFeatureSource();
  ogrSource->setURL(url);
  REQUIRE(ogrSource->open().isOK());
  REQUIRE(bulletManager->createTerrain(ogrSource));
}

class SampleRigidUpdater : public osgGA::GUIEventHandler {
  typedef std::map<btRigidBody *, osg::MatrixTransform *> MTNodeMap;
  MTNodeMap _osgNodes;
  osg::observer_ptr<osg::Group> _root;
  BulletManager *_bulletManager;
 public:
  SampleRigidUpdater(osg::Group *group, BulletManager *manager) : _root(group), _bulletManager(manager) {}

  void addGround() {
	osg::ref_ptr<osg::MatrixTransform> groundMT = new osg::MatrixTransform;
	osg::ref_ptr<osg::Geode> groundGeode = new osg::Geode; // 地理 node
	osg::ref_ptr<osg::Box> groundShape = new osg::Box(osg::Vec3(0.0f, 0.0f, -0.5f),
													  100.0f,
													  100.0f,
													  1.0f);
	osg::ref_ptr<osg::ShapeDrawable> groundDrawable = new osg::ShapeDrawable(groundShape);
	groundGeode->addDrawable(groundDrawable);
	groundMT->addChild(groundGeode.get());
	_root->addChild(groundMT.get());
  }

  btRigidBody *addPhysicsBox(osg::Box *box, const osg::Vec3 &pos, const osg::Vec3 &vel, float mass) {
	const osg::Vec3 &dim = box->getHalfLengths();
	btCollisionShape *boxShape = new btBoxShape(btVector3(dim[0], dim[1], dim[2]));
	btTransform boxTransform;
	boxTransform.setIdentity();

	btVector3 localInertia(0.0, 0.0, 0.0);
	if (mass > 0.0) {
	  boxShape->calculateLocalInertia(mass, localInertia);
	}

	auto *motionState = new btDefaultMotionState(boxTransform);
	btRigidBody::btRigidBodyConstructionInfo rigidInfo(mass, motionState, boxShape, localInertia);
	auto *pRigidBody = new btRigidBody(rigidInfo);
	_bulletManager->getDynamicsWorld()->addRigidBody(pRigidBody);

	// 物理位置
	btTransform trans;
	trans.setFromOpenGLMatrix(osg::Matrixf(osg::Matrix::translate(pos)).ptr());
	pRigidBody->setWorldTransform(trans);

	// 物理速度
	pRigidBody->setLinearVelocity(btVector3(vel.x(), vel.y(), vel.z()));

	return pRigidBody;
  }

  bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) override {
	auto *view = dynamic_cast<osgViewer::View *>( &aa );

	if (!view || !_root) return false;

	switch (ea.getEventType()) {
	  case osgGA::GUIEventAdapter::PUSH:
		if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) {
		  osg::Vec3 eye, center, up, dir;
		  view->getCamera()->getViewMatrixAsLookAt(eye, center, up);
		  OSG_ALWAYS << "Fire: " << eye << std::endl;
		  dir = center - eye;
		  dir.normalize();
		  auto *pShape = new osg::Box(osg::Vec3(), 100.0f);
		  btRigidBody *body = addPhysicsBox(pShape, eye+dir*10, dir*1000.0f, 2.0);
		  addOSGShape(body, pShape);
		}
		break;
	  case osgGA::GUIEventAdapter::FRAME:
		_bulletManager->getDynamicsWorld()->stepSimulation(1.f / 60.f, 10);
		for (auto &itr : _osgNodes) {
		  osg::Matrix matrix = bulletManager->getRigidBodyMatrixByKey(itr.second); // 获取在物理引擎中的矩阵
		  itr.second->setMatrix(matrix); // 设置矩阵到 OSG 场景的对应 Node
		  // osg::notify(osg::ALWAYS) << matrix << std::endl;
		}
		break;
	  default:break;
	}
	return false;
  }

  void addOSGNode(btRigidBody *body, osg::MatrixTransform *mt) {
	_osgNodes[body] = mt;
	_bulletManager->registerRigidBodyAndGetKey(mt, body);
  }

  void addOSGShape(btRigidBody *body, osg::Shape *shape) {
	osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(new osg::ShapeDrawable(shape));
	mt->addChild(geode.get());
	_root->addChild(mt.get());
	_osgNodes[body] = mt;
	_bulletManager->registerRigidBodyAndGetKey(mt.get(), body);
  }
} *rigidUpdater = nullptr;

TEST_CASE("bullet & osg & osgEarth") {
  osg::notify(osg::ALWAYS) << "StartWorldPos: " << tank->getStartWorldPos() << std::endl;
  rigidUpdater = new SampleRigidUpdater(root, bulletManager);
  // 暂时手动控制
  // btRigidBody *body = rigidUpdater->addPhysicsBox(tank->getPureBoundingBox(),
  // 											  tank->getStartWorldPos(),
  // 											  {0, 0, 0},
  // 											  0);
  // rigidUpdater->addOSGNode(body, tank->getTankMatrixTransform());
  // bulletManager->registerRigidBodyAndGetKey(tank->getTankMatrixTransform(), body);

  // 设置重力
  osg::Vec3d geocentricXYZ = mapLayerManager->getEarthEllipsoid().geodeticToGeocentric(lonLatAlt);
  geocentricXYZ.normalize();
  geocentricXYZ = -geocentricXYZ*9.8;
  bulletManager->setGravity(osg::Vec3(geocentricXYZ));

  // 物理模拟循环
  viewer->addEventHandler(rigidUpdater);

  // 载具事件循环
  viewer->addEventHandler(tank->moveGUIEventHandler);
  // viewer->addEventHandler(spaceShip->moveGUIEventHandler);
}

int main(int argc, char **argv) {
  osgEarth::initialize();
  // osg::setNotifyLevel(osg::NotifySeverity::DEBUG_FP);
  // osgEarth::setNotifyLevel(osg::NotifySeverity::DEBUG_FP);
  osg::ArgumentParser arguments(&argc, argv);
  bulletManager = new BulletManager;
  root = new osg::Group;
  em = new EarthManipulator();
//    rigidUpdater->addGround();
//    for (unsigned int i = 0; i < 10; ++i) {
//        for (unsigned int j = 0; j < 10; ++j) {
//            auto *pShape = new osg::Box(osg::Vec3(), 0.99f);
//            int key = rigidUpdater->addPhysicsBox(pShape,
//                                                  osg::Vec3((float) i, 0.0f, (float) j + 0.5f), osg::Vec3(), 1.0f);
//            rigidUpdater->addOSGShape(key, pShape);
//        }
//    }
  viewer = new osgViewer::Viewer(arguments);
  // viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));
  viewer->addEventHandler(new osgViewer::StatsHandler);
  viewer->addEventHandler(new osgViewer::WindowSizeHandler);
  viewer->setSceneData(root);
  viewer->setCameraManipulator(em);
  // viewer->setUpViewInWindow(0, 0, 1024, 960);
  // viewer->getCamera()->setSmallFeatureCullingPixelSize(-1.0f);
  // GLUtils::setGlobalDefaults(viewer->getCamera()->getOrCreateStateSet());
  // osg::ref_ptr<osg::StateSet> state = root->getOrCreateStateSet();
  // state->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
  Catch::Session().run(argc, argv);
  return viewer->run();
}