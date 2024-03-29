//
// Created by TictorDC on 2022/7/12.
//

#include <osg/ComputeBoundsVisitor>
#include <osg/TriangleFunctor>
#include <osg/io_utils>
#include <osgEarth/Progress>
#include <osgEarth/FeatureNode>
#include <osgEarthDrivers/engine_rex/TileNode>

#include "BulletManager.h"
#include "TriangleMeshVisitor.h"

using namespace osgEarth;

namespace MultiLayerTileMap {

BulletManager::BulletManager(osgEarth::MapNode *node) : mapNode(node) {
  collisionConfiguration = new btDefaultCollisionConfiguration;
  dispatcher = new btCollisionDispatcher(collisionConfiguration);
  pairCache = new btDbvtBroadphase;
  solver = new btSequentialImpulseConstraintSolver;
  dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, pairCache, solver, collisionConfiguration);
}

btDiscreteDynamicsWorld *BulletManager::getDynamicsWorld() const {
  return dynamicsWorld;
}

BulletManager::~BulletManager() {
  // 将刚体从 dynamics 世界中移除，并删除它们
  if (dynamicsWorld != nullptr) {
	for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
	  auto *obj = dynamicsWorld->getCollisionObjectArray()[i];
	  auto *body = btRigidBody::upcast(obj);
	  if (body && body->getMotionState()) {
		delete body->getMotionState();
	  }
	  dynamicsWorld->removeCollisionObject(obj);
	  delete obj;
	}
  }
  for (auto &it : collisionShapeMap) {
	delete it.second;
  }
  // 删除 dynamics 世界
  delete dynamicsWorld;
  // 删除解算器
  delete solver;
  // 删除宽相
  delete pairCache;
  // 删除调度器
  delete dispatcher;
  // 删除刚体配置
  delete collisionConfiguration;
}

btBoxShape *BulletManager::btBoxCollisionShapeFromOSG(osg::Node *node, const osg::BoundingBox *bb) {
  osg::BoundingBox bbox;
  if (bb) {
	bbox = *bb;
  } else {
	osg::ComputeBoundsVisitor visitor;
	node->accept(visitor);
	bbox = visitor.getBoundingBox();
  }

  auto shape = new btBoxShape(btVector3((bbox.xMax() - bbox.xMin()) * 0.5f,
										(bbox.yMax() - bbox.yMin()) * 0.5f,
										(bbox.zMax() - bbox.zMin()) * 0.5f));
  return shape;
}

btRigidBody *BulletManager::createSimplePlaneForDebug(const osg::Plane &plane) {
  if (dynamicsWorld == nullptr) { // 物理世界未初始化
	return nullptr;
  }
  osg::Vec3 norm = plane.getNormal();
  auto *groundShape = new btStaticPlaneShape(btVector3(norm[0], norm[1], norm[2]),
											 (float)plane[3]);
  btTransform groundTransform; // 旋转和移动
  groundTransform.setIdentity(); // 旋转矩阵为单位矩阵，不移动
  auto *motionState = new btDefaultMotionState(groundTransform);
  btRigidBody::btRigidBodyConstructionInfo groundRigidInfo(0.0, // 质量
														   motionState,
														   groundShape, // 碰撞形状
														   btVector3(0.0, 0.0, 0.0)); // 惯性
  auto *groundBody = new btRigidBody(groundRigidInfo);
  dynamicsWorld->addRigidBody(groundBody);
  return groundBody;
}

const btCollisionShape *BulletManager::getCollisionShapeByKey(const osg::Node *pNode) {
  if (pNode == nullptr) {
	return nullptr;
  }
  return collisionShapeMap[pNode];
}

osg::Matrix BulletManager::getRigidBodyMatrixByKey(const osg::Node *pNode) {
  if (pNode == nullptr || rigidBodyMap.find(pNode) == rigidBodyMap.end()) {
	return {};
  }
  auto *body = rigidBodyMap[pNode];
  btTransform trans;
  trans = body->getWorldTransform();
  // if (body && body->getMotionState()) {
  //     body->getMotionState()->getWorldTransform(trans);
  // } else {
  //     trans = body->getWorldTransform();
  // }
  osg::Matrixf matrix;
  trans.getOpenGLMatrix(matrix.ptr());
  return matrix;
}

const osg::Node *BulletManager::registerRigidBodyAndGetKey(const osg::Node *pNode, const btRigidBody *pRigidBody) {
  if (rigidBodyMap.find(pNode) != rigidBodyMap.end()) {
	// 存在旧的刚体，尝试删除
	const btRigidBody *pOldBody = rigidBodyMap[pNode];
	dynamicsWorld->removeRigidBody(const_cast<btRigidBody *>(pOldBody));
	delete pOldBody;
  }
  dynamicsWorld->addRigidBody(const_cast<btRigidBody *>(pRigidBody));
  rigidBodyMap[pNode] = pRigidBody;
  return pNode;
}

btTransform BulletManager::asBtTransform(const osg::Matrix &matrix) {
  /// \note: osg::Matrix is ref of osg::Matrixd now, which means double cast to float below
  const osg::Matrix::value_type *oPtr = matrix.ptr();
  btScalar bPtr[16];
  for (int idx = 0; idx < 16; idx++) {
	bPtr[idx] = (float)oPtr[idx];
  }
  btTransform t;
  t.setFromOpenGLMatrix(bPtr);
  return t;
}

int BulletManager::getSelectedTerrainTileLod() const {
  return selectedTerrainTileLOD;
}

void BulletManager::setSelectedTerrainTileLod(int selectedTerrainTileLod) {
  selectedTerrainTileLOD = selectedTerrainTileLod;
}

btBvhTriangleMeshShape *BulletManager::asBtBvhTriangleMeshShape(const osg::Vec3Array &vertices) {
  auto *mesh = new btTriangleMesh;
  for (size_t i = 0; i + 2 < vertices.size(); i += 3) {
	const osg::Vec3f &p1 = vertices[i];
	const osg::Vec3f &p2 = vertices[i + 1];
	const osg::Vec3f &p3 = vertices[i + 2];
	mesh->addTriangle(btVector3(p1.x(), p1.y(), p1.z()),
					  btVector3(p2.x(), p2.y(), p2.z()),
					  btVector3(p3.x(), p3.y(), p3.z()));
  }
  OSG_ALWAYS << "Num of Triangles: " << mesh->getNumTriangles() << std::endl;

  auto *meshShape = new btBvhTriangleMeshShape(mesh, true);
  return meshShape;
}

btBvhTriangleMeshShape *BulletManager::asBtBvhTriangleMeshShape(std::vector<osg::Vec3> &vertices) {
  auto *mesh = new btTriangleMesh;
  for (size_t i = 0; i + 2 < vertices.size(); i += 3) {
	const osg::Vec3f &p1 = vertices[i];
	const osg::Vec3f &p2 = vertices[i + 1];
	const osg::Vec3f &p3 = vertices[i + 2];
	mesh->addTriangle(btVector3(p1.x(), p1.y(), p1.z()),
					  btVector3(p2.x(), p2.y(), p2.z()),
					  btVector3(p3.x(), p3.y(), p3.z()));
  }

  auto *meshShape = new btBvhTriangleMeshShape(mesh, true);
  return meshShape;
}

const osg::Node *BulletManager::createTerrainTile(const TileKey &tileKey,
												  osg::Node *graph,
												  TerrainCallbackContext &context) {
  if (dynamicsWorld == nullptr || mapNode == nullptr) { // 物理世界和 OSG 世界未初始化
	return nullptr;
  }
  /// 这是获取顶点的方法之一，另一种方法是直接访问 TileDrawable 的 _geom 成员 (public)
  /// \code
  /// auto *tileNode = dynamic_cast<osgEarth::REX::TileNode*>(graph);
  /// auto *surfaceNode = tileNode->getSurfaceNode();
  /// auto matrix = surfaceNode->getMatrix();
  /// auto trans = BulletManager::asBtTransform(matrix);
  /// auto shape = surfaceNode->getDrawable()->getShape();
  /// auto kdTree = dynamic_cast<osg::KdTree*>(shape);
  /// const osg::Vec3Array* vertices = kdTree->getVertices();
  /// \endcode
  /// KdTree 的 vertices 的长度大约恒定为 289
  ///  以下是获取顶点的另一种方法
  auto *tileNode = dynamic_cast<osgEarth::REX::TileNode *>(graph);
  auto *surfaceNode = tileNode->getSurfaceNode();
  auto matrix = surfaceNode->getMatrix();
  auto trans = asBtTransform(matrix);
  auto shape = surfaceNode->getDrawable()->_geom;

  osg::TriangleFunctor<TriangleMeshFunc> tf;
  shape->accept(tf);
  auto vertices = tf.vertices.get();
  /// 以下是遍历输出顶点的令一种方法
  /// \code
  /// auto vertices = shape->getVertexArray();
  /// osg::TemplatePrimitiveFunctor<NormalPrint> tf;
  /// shape->accept(tf);
  /// \endcode

  auto *btShape = asBtBvhTriangleMeshShape(*vertices);
  auto key = tileKey.str();

  auto *motionState = new btDefaultMotionState(trans);
  btRigidBody::btRigidBodyConstructionInfo rigidInfo(0.0, // 质量
													 motionState,
													 btShape, // 碰撞形状
													 btVector3(0.0, 0.0, 0.0)); // 惯性
  auto *rigidBody = new btRigidBody(rigidInfo);
  dynamicsWorld->addRigidBody(rigidBody);
  if (tileKey2NodeMap.find(key) != tileKey2NodeMap.end()) {
	// 瓦片已经存在，瓦片刚体可能已经存在，先尝试删除
	osg::Node *oldTileNode = tileKey2NodeMap[key];
	// TODO: 要不要删除旧的 osg::Node？瓦片引擎可能已经删除了
	if (rigidBodyMap.find(oldTileNode) != rigidBodyMap.end()) {
	  // 删除旧瓦片刚体
	  const btRigidBody *oldTileRigidBody = rigidBodyMap[oldTileNode];
	  dynamicsWorld->removeRigidBody(const_cast<btRigidBody *>(oldTileRigidBody));
	  delete oldTileRigidBody;
	}
  }
  // 由 tileKey 可索引到 Node
  tileKey2NodeMap[key] = graph;
  // 由 Node 可索引到刚体
  rigidBodyMap[graph] = rigidBody;

  return graph;
}

const osg::Node *BulletManager::createTerrainByOGRFeatureSource(osgEarth::OGRFeatureSource *featureSource) {
  for (int i = 0; i < featureSource->getFeatureCount(); i++) {
	createTerrainByFeature(featureSource->getFeature(i));
  }
  return nullptr;
}

osg::Node *BulletManager::createTerrainByFeature(osgEarth::Feature *feature) {
  Style geomStyle;
  /// \code
  /// geomStyle.getOrCreate<AltitudeSymbol>()->clamping() = AltitudeSymbol::CLAMP_TO_TERRAIN;
  /// geomStyle.getOrCreate<AltitudeSymbol>()->technique() = AltitudeSymbol::TECHNIQUE_DRAPE;
  /// \endcode
  /// 经过尝试发现只有这样矢量图形才能完好的贴地
  geomStyle.getOrCreate<AltitudeSymbol>()->clamping() = AltitudeSymbol::CLAMP_TO_TERRAIN;
  geomStyle.getOrCreate<AltitudeSymbol>()->technique() = AltitudeSymbol::TECHNIQUE_DRAPE;
  geomStyle.getOrCreate<AltitudeSymbol>()->binding() = AltitudeSymbol::BINDING_VERTEX; // 矢量文件的每个顶点独立贴合
  geomStyle.getOrCreate<PolygonSymbol>()->fill()->color() = Color(Color::Cyan, 0.4f);
  geomStyle.getOrCreate<PolygonSymbol>()->outline() = true;
  geomStyle.getOrCreate<LineSymbol>()->stroke()->color() = Color::Lime;
  geomStyle.getOrCreate<LineSymbol>()->stroke()->width() = 8.0f;
  geomStyle.getOrCreate<LineSymbol>()->stroke()->widthUnits() = Units::METERS;
  geomStyle.getOrCreate<RenderSymbol>()->depthOffset()->enabled() = true;
  osgEarth::FeatureNode *featureNode = new osgEarth::FeatureNode(feature, geomStyle);
  featureNode->setMapNode(mapNode);
  mapNode->addChild(featureNode);
  featureNode->dirty();

  createTerrainByFeatureNode(featureNode);

  return featureNode;
}
osg::Node *BulletManager::createTerrainByFeatureNode(osgEarth::FeatureNode *featureNode) {
  FeatureNodeVisitor fNV;
  // 获取顶点
  featureNode->accept(fNV);
  // 合并分支
  fNV.mergeAllFeature();
  // 重新计算中心点
  fNV.reCalcCentroid();
  // 获取位置矩阵
  btTransform transform = fNV.getTransform();
  OSG_ALWAYS << transform << std::endl;

  btBvhTriangleMeshShape *btShape = asBtBvhTriangleMeshShape(*fNV.mergedFeature);
  auto *motionState = new btDefaultMotionState(transform);
  btRigidBody::btRigidBodyConstructionInfo rigidInfo(0.0, // 质量
													 motionState,
													 btShape, // 碰撞形状
													 btVector3(0.0, 0.0, 0.0)); // 惯性
  rigidInfo.m_friction = 1000; // 摩擦
  rigidInfo.m_rollingFriction = 1000; // 滚动摩擦
  rigidInfo.m_restitution = 0; // 恢复系数
  rigidInfo.m_linearDamping = 1000; // 线性阻尼
  rigidInfo.m_angularDamping = 1000; // 角阻尼

  auto *rigidBody = new btRigidBody(rigidInfo);
  registerRigidBodyAndGetKey(featureNode, rigidBody);
  // OSG_ALWAYS << rigidBody->getWorldTransform() << std::endl;
  // OSG_ALWAYS << rigidBody->getCenterOfMassPosition() << std::endl;
  return featureNode;
}

void BulletManager::BulletTerrainChangedCallback::onTileUpdate(const osgEarth::TileKey &tileKey,
															   osg::Node *graph,
															   osgEarth::TerrainCallbackContext &context) {
  if (_bulletManager != nullptr) {
	auto lod = tileKey.getLOD();
	osg::notify(osg::ALWAYS) << "LOD: " << lod << std::endl;
	if (lod != _bulletManager->getSelectedTerrainTileLod()) {
	  return;
	}
	onTileUpdateTimeTick = osg::Timer::instance()->tick();
	osg::notify(osg::DEBUG_INFO) << "TileUpdate tick: " << onTileUpdateTimeTick << std::endl;

	_bulletManager->createTerrainTile(tileKey, graph, context);
  }
}
} // MultiLayerTileMap