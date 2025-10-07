#include "wrapper.h"
#include "PxPhysicsAPI.h"
#include <vector>

using namespace physx;

class SimpleErrorCallback : public PxErrorCallback {
public:
	virtual void reportError(PxErrorCode::Enum code, const char* message,
		const char* file, int line) override {
		fprintf(stderr, "PhysX Error [%d]: %s at %s:%d\n", code, message, file, line);
	}
};

static SimpleErrorCallback gErrorCallback;
static PxDefaultAllocator gAllocator;

extern "C" {

	PxGoFoundationHandle PxGoCreateFoundation(uint32_t version, const char* allocatorName) {
		return (PxGoFoundationHandle)PxCreateFoundation(version, gAllocator, gErrorCallback);
	}

	void PxGoReleaseFoundation(PxGoFoundationHandle foundation) {
		if (foundation) {

			try {
				((PxFoundation*)foundation)->release();
			}
			catch (...) {
				printf("Exception in PxGoReleaseFoundation!\n");
			}

		}
	}

	PxGoPvdHandle PxGoCreatePvd(PxGoFoundationHandle foundationHandle)
	{
		PxFoundation* foundation = (PxFoundation*)foundationHandle;
		if (!foundation)
			return nullptr;
		PxPvd* pvd = PxCreatePvd(*foundation);
		return (PxGoPvdHandle)pvd;
	}

	bool PxGoConnectPvd(PxGoPvdHandle pvdHandle, const char* host, int port)
	{
		PxPvd* pvd = (PxPvd*)pvdHandle;
		if (!pvd || !host || !*host)
			return false;

		PxPvdTransport* transport =
			PxDefaultPvdSocketTransportCreate(host, port > 0 ? port : 5425, 10);
		if (!transport)
			return false;

		bool connected = pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
		if (!connected)
		{
			transport->release();
			return false;
		}
		return true;
	}

	void PxGoReleasePvd(PxGoPvdHandle pvdHandle)
	{
		PxPvd* pvd = (PxPvd*)pvdHandle;
		if (!pvd)
			return;
		try {
			PxPvdTransport* transport = pvd->getTransport();
			pvd->release();
			if (transport)
				transport->release();
		}
		catch (...) {
			printf("Exception in PxGoReleasePvd!\n");
		}

	}

	PxGoCollectionHandle PxGoLoadCollectionFromXmlFile(
		const char* path,
		PxGoPhysicsHandle physicsHandle,
		PxGoCookingHandle cookingHandle)
	{
		if (!path || !*path || !physicsHandle)
			return nullptr;

		PxPhysics* physics = (PxPhysics*)physicsHandle;
		PxCooking* cooking = (PxCooking*)cookingHandle;
		PxSerializationRegistry* registry = PxSerialization::createSerializationRegistry(*physics);

		// 读取文件
		PxDefaultFileInputData input(path);
		if (!input.isValid())
			return nullptr;

		// 解析 XML 并创建 Collection
		PxStringTable* stringTable = &PxStringTableExt::createStringTable(gAllocator);
		PxCollection* collection = PxSerialization::createCollectionFromXml(input, *cooking, *registry, 0, stringTable);

		registry->release();
		stringTable->release();
		return (PxGoCollectionHandle)collection;
	}

	PxGoCollectionHandle PxGoLoadCollectionFromXmlMemory(
		const char* xmlData,
		size_t xmlSize,
		PxGoPhysicsHandle physicsHandle,
		PxGoCookingHandle cookingHandle)
	{
		if (!xmlData || xmlSize == 0 || !physicsHandle)
			return nullptr;

		PxPhysics* physics = (PxPhysics*)physicsHandle;
		PxCooking* cooking = (PxCooking*)cookingHandle;
		PxSerializationRegistry* registry = PxSerialization::createSerializationRegistry(*physics);
		PxDefaultMemoryInputData input((PxU8*)xmlData, xmlSize);

		// 解析 XML 并创建 Collection
		PxStringTable* stringTable = &PxStringTableExt::createStringTable(gAllocator);
		PxCollection* collection = PxSerialization::createCollectionFromXml(input, *cooking, *registry, 0, stringTable);

		registry->release();
		stringTable->release();
		return (PxGoCollectionHandle)collection;
	}

	void PxGoReleaseCollection(PxGoCollectionHandle collectionHandle)
	{
		try {
			PxCollection* collection = (PxCollection*)collectionHandle;
			if (collection)
				collection->release();
		}
		catch (...) {
			printf("Exception in PxGoReleaseCollection!\n");
		}

	}


	PxGoPhysicsHandle PxGoCreatePhysics(
		uint32_t version,
		PxGoFoundationHandle foundationHandle,
		float toleranceScale,
		PxGoPvdHandle pvdHandle)
	{
		PxFoundation* foundation = (PxFoundation*)foundationHandle;
		if (!foundation)
			return nullptr;

		PxTolerancesScale scale;
		scale.length = toleranceScale;
		scale.speed = 10.0f;

		// 创建 Physics
		PxPhysics* physics = PxCreatePhysics(version, *foundation, scale, true, (PxPvd*)pvdHandle);
		if (!physics)
			return nullptr;

		return (PxGoPhysicsHandle)physics;
	}



	void PxGoReleasePhysics(PxGoPhysicsHandle physics) {
		if (physics) {
			try {
				((PxPhysics*)physics)->release();
			}
			catch (...) {
				printf("Exception in PxGoReleasePhysics!\n");
			}

		}
	}

	PxGoCookingHandle PxGoCreateCooking(uint32_t version, PxGoFoundationHandle foundation) {
		PxTolerancesScale scale;
		PxCookingParams params(scale);
		return (PxGoCookingHandle)PxCreateCooking(version, *(PxFoundation*)foundation, params);
	}

	void PxGoReleaseCooking(PxGoCookingHandle cooking) {
		if (cooking) {

			try {
				((PxCooking*)cooking)->release();
			}
			catch (...) {
				printf("Exception in PxGoReleaseCooking!\n");
			}
		}
	}

	PxGoSceneHandle PxGoCreateScene(PxGoPhysicsHandle physics, PxGoSceneDesc* desc) {
		PxPhysics* px = (PxPhysics*)physics;
		PxSceneDesc sceneDesc(px->getTolerancesScale());
		sceneDesc.gravity = PxVec3(desc->gravity.x, desc->gravity.y, desc->gravity.z);
		sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(2);
		sceneDesc.filterShader = PxDefaultSimulationFilterShader;
		sceneDesc.flags |= desc->enableCCD ? PxSceneFlag::eENABLE_CCD : PxSceneFlag::Enum(0);

		return (PxGoSceneHandle)px->createScene(sceneDesc);
	}

	void PxGoReleaseScene(PxGoSceneHandle scene) {
		if (scene) {
			try {
				((PxScene*)scene)->release();
			}
			catch (...) {
				printf("Exception in PxGoReleaseScene!\n");
			}
		}
	}

	void PxGoSceneSimulate(PxGoSceneHandle scene, float dt) {
		if (scene) {
			((PxScene*)scene)->simulate(dt);
		}
	}

	bool PxGoSceneFetchResults(PxGoSceneHandle scene, bool block) {
		if (scene) {
			return ((PxScene*)scene)->fetchResults(block);
		}
		return false;
	}

	void PxGoSceneAddActor(PxGoSceneHandle scene, PxGoRigidDynamicHandle actor) {
		if (scene && actor) {
			((PxScene*)scene)->addActor(*(PxRigidDynamic*)actor);
		}
	}

	void PxGoSceneRemoveActor(PxGoSceneHandle scene, PxGoRigidDynamicHandle actor) {
		if (scene && actor) {
			((PxScene*)scene)->removeActor(*(PxRigidDynamic*)actor);
		}
	}

	void PxGoSceneAddStaticActor(PxGoSceneHandle scene, PxGoRigidStaticHandle actor) {
		if (scene && actor) {
			((PxScene*)scene)->addActor(*(PxRigidStatic*)actor);
		}
	}

	PxGoRigidStaticHandle PxGoSceneCreateStaticActorFromCollection(
		PxGoSceneHandle sceneHandle,
		PxGoCollectionHandle collectionHandle,
		uint32_t index,
		PxGoTransform* transform)
	{
		if (!sceneHandle || !collectionHandle)
			return nullptr;

		PxScene* scene = reinterpret_cast<PxScene*>(sceneHandle);
		PxCollection* collection = reinterpret_cast<PxCollection*>(collectionHandle);

		PxBase* obj = collection->find(index);
		if (!obj)
			return nullptr;

		// 检查类型是否为 PxRigidActor
		if (!obj->is<PxRigidActor>())
			return nullptr;

		PxRigidActor* collection_actor = static_cast<PxRigidActor*>(obj);
		PxTransform t(PxVec3(transform->p.x, transform->p.y, transform->p.z),
			PxQuat(transform->q.x, transform->q.y, transform->q.z, transform->q.w));

		auto actor = PxCloneStatic(scene->getPhysics(), t, *collection_actor);
		// 如果该 actor 未添加到场景，则添加进去
		bool alreadyInScene = false;
		{
			PxScene* existingScene = actor->getScene();
			if (existingScene != nullptr && existingScene == scene)
				alreadyInScene = true;
		}

		if (!alreadyInScene) {
			scene->addActor(*actor);
		}

		return reinterpret_cast<PxGoRigidStaticHandle>(actor);
	}

	PxGoRigidDynamicHandle PxGoSceneCreateKinematicActorFromCollection(
		PxGoSceneHandle sceneHandle,
		PxGoCollectionHandle collectionHandle,
		uint32_t index,
		PxGoTransform* transform)
	{
		if (!sceneHandle || !collectionHandle)
			return nullptr;

		PxScene* scene = reinterpret_cast<PxScene*>(sceneHandle);
		PxCollection* collection = reinterpret_cast<PxCollection*>(collectionHandle);

		PxBase* obj = collection->find(index);
		if (!obj)
			return nullptr;

		// 检查类型是否为 PxRigidActor
		if (!obj->is<PxRigidActor>())
			return nullptr;

		PxRigidActor* collection_actor = static_cast<PxRigidActor*>(obj);
		PxTransform t(PxVec3(transform->p.x, transform->p.y, transform->p.z),
			PxQuat(transform->q.x, transform->q.y, transform->q.z, transform->q.w));

		std::vector<PxShape*> shapes(64);
		shapes.resize(collection_actor->getNbShapes());

		PxU32 shapeCount = collection_actor->getNbShapes();
		collection_actor->getShapes(shapes.data(), shapeCount);

		PxRigidDynamic* actor = PxCreateKinematic(scene->getPhysics(), t, *shapes[0], 1);
		if (!actor)
			return nullptr;

		shapes[0]->release();
		for (PxU32 i = 1; i < shapeCount; i++)
		{
			PxShape* s = shapes[i];
			if (!s->isExclusive())
				actor->attachShape(*s);
			else
			{
				PxShape* newShape = physx::PxCloneShape(scene->getPhysics(), *s, true);
				actor->attachShape(*newShape);
				newShape->release();
			}
		}

		actor->setActorFlags(collection_actor->getActorFlags());
		actor->setOwnerClient(collection_actor->getOwnerClient());
		actor->setClientBehaviorFlags(collection_actor->getClientBehaviorFlags());
		actor->setDominanceGroup(collection_actor->getDominanceGroup());

		// 如果该 actor 未添加到场景，则添加进去
		bool alreadyInScene = false;
		{
			PxScene* existingScene = actor->getScene();
			if (existingScene != nullptr && existingScene == scene)
				alreadyInScene = true;
		}

		if (!alreadyInScene) {
			scene->addActor(*actor);
		}

		return reinterpret_cast<PxGoRigidDynamicHandle>(actor);
	}

	void copyStaticProperties(PxPhysics& physics, PxRigidActor& to, const PxRigidActor& from)
	{
		std::vector<PxShape*> shapes(64);
		shapes.resize(from.getNbShapes());

		PxU32 shapeCount = from.getNbShapes();
		from.getShapes(shapes.data(), shapeCount);

		for (PxU32 i = 0; i < shapeCount; i++)
		{
			PxShape* s = shapes[i];
			if (!s->isExclusive())
				to.attachShape(*s);
			else
			{
				PxShape* newShape = physx::PxCloneShape(physics, *s, true);
				to.attachShape(*newShape);
				newShape->release();
			}
		}

		to.setActorFlags(from.getActorFlags());
		to.setOwnerClient(from.getOwnerClient());
		to.setClientBehaviorFlags(from.getClientBehaviorFlags());
		to.setDominanceGroup(from.getDominanceGroup());
	}

	PxGoRigidDynamicHandle PxGoSceneCreateDynamicActorFromCollection(
		PxGoSceneHandle sceneHandle,
		PxGoCollectionHandle collectionHandle,
		uint32_t index,
		PxGoTransform* transform)
	{
		if (!sceneHandle || !collectionHandle)
			return nullptr;

		PxScene* scene = reinterpret_cast<PxScene*>(sceneHandle);
		PxCollection* collection = reinterpret_cast<PxCollection*>(collectionHandle);

		PxBase* obj = collection->find(index);
		if (!obj)
			return nullptr;

		// 检查类型是否为 PxRigidDynamic
		if (!obj->is<PxRigidActor>())
			return nullptr;

		PxRigidActor* collection_actor = static_cast<PxRigidActor*>(obj);
		auto nb = collection_actor->getNbShapes();
		if (nb == 0) {
			return nullptr;
		}
		PxTransform t(PxVec3(transform->p.x, transform->p.y, transform->p.z),
			PxQuat(transform->q.x, transform->q.y, transform->q.z, transform->q.w));

		PxRigidDynamic* actor = scene->getPhysics().createRigidDynamic(t);
		if (!actor)
			return nullptr;
		copyStaticProperties(scene->getPhysics(), *actor, *collection_actor);

		// 如果该 actor 未添加到场景，则添加进去
		bool alreadyInScene = false;
		{
			PxScene* existingScene = actor->getScene();
			if (existingScene != nullptr && existingScene == scene)
				alreadyInScene = true;
		}

		if (!alreadyInScene) {
			scene->addActor(*actor);
		}

		return reinterpret_cast<PxGoRigidDynamicHandle>(actor);
	}

	void PxGoSceneRemoveStaticActor(PxGoSceneHandle scene, PxGoRigidStaticHandle actor) {
		if (scene && actor) {
			((PxScene*)scene)->removeActor(*(PxRigidStatic*)actor);
		}
	}

	PxGoMaterialHandle PxGoCreateMaterial(PxGoPhysicsHandle physics, float staticFriction,
		float dynamicFriction, float restitution) {
		if (physics) {
			return (PxGoMaterialHandle)((PxPhysics*)physics)->createMaterial(
				staticFriction, dynamicFriction, restitution);
		}
		return nullptr;
	}

	void PxGoReleaseMaterial(PxGoMaterialHandle material) {
		if (material) {
			((PxMaterial*)material)->release();
		}
	}

	PxGoShapeHandle PxGoCreateShapeSphere(PxGoPhysicsHandle physics, PxGoSphereGeometry* geometry,
		PxGoMaterialHandle material, bool isExclusive) {
		if (physics && geometry && material) {
			PxSphereGeometry geo(geometry->radius);
			return (PxGoShapeHandle)((PxPhysics*)physics)->createShape(
				geo, *(PxMaterial*)material, isExclusive);
		}
		return nullptr;
	}

	PxGoShapeHandle PxGoCreateShapeBox(PxGoPhysicsHandle physics, PxGoBoxGeometry* geometry,
		PxGoMaterialHandle material, bool isExclusive) {
		if (physics && geometry && material) {
			PxBoxGeometry geo(geometry->halfExtents.x, geometry->halfExtents.y,
				geometry->halfExtents.z);
			return (PxGoShapeHandle)((PxPhysics*)physics)->createShape(
				geo, *(PxMaterial*)material, isExclusive);
		}
		return nullptr;
	}

	PxGoShapeHandle PxGoCreateShapeCapsule(PxGoPhysicsHandle physics, PxGoCapsuleGeometry* geometry,
		PxGoMaterialHandle material, bool isExclusive) {
		if (physics && geometry && material) {
			PxCapsuleGeometry geo(geometry->radius, geometry->halfHeight);
			return (PxGoShapeHandle)((PxPhysics*)physics)->createShape(
				geo, *(PxMaterial*)material, isExclusive);
		}
		return nullptr;
	}

	void PxGoReleaseShape(PxGoShapeHandle shape) {
		if (shape) {
			((PxShape*)shape)->release();
		}
	}

	PxGoRigidDynamicHandle PxGoCreateRigidDynamic(PxGoPhysicsHandle physics, PxGoTransform* transform) {
		if (physics && transform) {
			PxTransform t(PxVec3(transform->p.x, transform->p.y, transform->p.z),
				PxQuat(transform->q.x, transform->q.y, transform->q.z, transform->q.w));
			return (PxGoRigidDynamicHandle)((PxPhysics*)physics)->createRigidDynamic(t);
		}
		return nullptr;
	}

	void PxGoReleaseRigidDynamic(PxGoRigidDynamicHandle actor) {
		if (actor) {

			try {
				((PxRigidDynamic*)actor)->release();
			}
			catch (...) {
				printf("Exception in PxGoReleaseRigidDynamic!\n");
			}

		}
	}

	void PxGoRigidDynamicAttachShape(PxGoRigidDynamicHandle actor, PxGoShapeHandle shape) {
		if (actor && shape) {
			((PxRigidDynamic*)actor)->attachShape(*(PxShape*)shape);
		}
	}

	void PxGoRigidDynamicSetMass(PxGoRigidDynamicHandle actor, float mass) {
		if (actor) {
			PxRigidBodyExt::setMassAndUpdateInertia(*(PxRigidDynamic*)actor, mass);
		}
	}

	void PxGoRigidDynamicSetLinearVelocity(PxGoRigidDynamicHandle actor, PxGoVec3* velocity) {
		if (actor && velocity) {
			((PxRigidDynamic*)actor)->setLinearVelocity(
				PxVec3(velocity->x, velocity->y, velocity->z));
		}
	}

	void PxGoRigidDynamicSetAngularVelocity(PxGoRigidDynamicHandle actor, PxGoVec3* velocity) {
		if (actor && velocity) {
			((PxRigidDynamic*)actor)->setAngularVelocity(
				PxVec3(velocity->x, velocity->y, velocity->z));
		}
	}

	void PxGoRigidDynamicGetGlobalPose(PxGoRigidDynamicHandle actor, PxGoTransform* transform) {
		if (actor && transform) {
			PxTransform t = ((PxRigidDynamic*)actor)->getGlobalPose();
			transform->p.x = t.p.x;
			transform->p.y = t.p.y;
			transform->p.z = t.p.z;
			transform->q.x = t.q.x;
			transform->q.y = t.q.y;
			transform->q.z = t.q.z;
			transform->q.w = t.q.w;
		}
	}

	void PxGoRigidDynamicSetGlobalPose(PxGoRigidDynamicHandle actor, PxGoTransform* transform) {
		if (actor && transform) {
			PxTransform t(PxVec3(transform->p.x, transform->p.y, transform->p.z),
				PxQuat(transform->q.x, transform->q.y, transform->q.z, transform->q.w));
			((PxRigidDynamic*)actor)->setGlobalPose(t);
		}
	}

	void PxGoRigidDynamicAddForce(PxGoRigidDynamicHandle actor, PxGoVec3* force, uint32_t mode) {
		if (actor && force) {
			((PxRigidDynamic*)actor)->addForce(
				PxVec3(force->x, force->y, force->z),
				(PxForceMode::Enum)mode);
		}
	}

	void PxGoRigidDynamicGetLinearVelocity(PxGoRigidDynamicHandle actor, PxGoVec3* velocity) {
		if (actor && velocity) {
			PxVec3 v = ((PxRigidDynamic*)actor)->getLinearVelocity();
			velocity->x = v.x;
			velocity->y = v.y;
			velocity->z = v.z;
		}
	}

	PxGoRigidStaticHandle PxGoCreateRigidStatic(PxGoPhysicsHandle physics, PxGoTransform* transform) {
		if (physics && transform) {
			PxTransform t(PxVec3(transform->p.x, transform->p.y, transform->p.z),
				PxQuat(transform->q.x, transform->q.y, transform->q.z, transform->q.w));
			return (PxGoRigidStaticHandle)((PxPhysics*)physics)->createRigidStatic(t);
		}
		return nullptr;
	}

	void PxGoReleaseRigidStatic(PxGoRigidStaticHandle actor) {
		if (actor) {

			try {
				((PxRigidStatic*)actor)->release();
			}
			catch (...) {
				printf("Exception in PxGoReleaseRigidStatic!\n");
			}
		}
	}

	void PxGoRigidStaticAttachShape(PxGoRigidStaticHandle actor, PxGoShapeHandle shape) {
		if (actor && shape) {
			((PxRigidStatic*)actor)->attachShape(*(PxShape*)shape);
		}
	}

	void PxGoRigidStaticGetGlobalPose(PxGoRigidStaticHandle actor, PxGoTransform* transform) {
		if (actor && transform) {
			PxTransform t = ((PxRigidStatic*)actor)->getGlobalPose();
			transform->p.x = t.p.x;
			transform->p.y = t.p.y;
			transform->p.z = t.p.z;
			transform->q.x = t.q.x;
			transform->q.y = t.q.y;
			transform->q.z = t.q.z;
			transform->q.w = t.q.w;
		}
	}

	void PxGoRigidDynamicSetKinematicTarget(PxGoRigidDynamicHandle actorHandle, PxGoTransform* target)
	{
		if (!actorHandle || !target) return;
		PxRigidDynamic* actor = reinterpret_cast<PxRigidDynamic*>(actorHandle);

		PxTransform pxTarget(
			PxVec3(target->p.x, target->p.y, target->p.z),
			PxQuat(target->q.x, target->q.y, target->q.z, target->q.w)
		);

		actor->setKinematicTarget(pxTarget);
	}

	PxGoVec3 PxGoVec3Make(float x, float y, float z) {
		PxGoVec3 v = { x, y, z };
		return v;
	}

	PxGoQuat PxGoQuatMake(float x, float y, float z, float w) {
		PxGoQuat q = { x, y, z, w };
		return q;
	}

	PxGoQuat PxGoQuatIdentity() {
		PxGoQuat q = { 0.0f, 0.0f, 0.0f, 1.0f };
		return q;
	}

	PxGoTransform PxGoTransformMake(PxGoVec3 position, PxGoQuat rotation) {
		PxGoTransform t = { position, rotation };
		return t;
	}

} // extern "C"