#ifndef PHYSX_C_API_H
#define PHYSX_C_API_H

#ifdef _WIN32
#ifdef PHYSX_WRAPPER_EXPORTS
#define PHYSX_GO_API __declspec(dllexport)
#elif defined(PHYSX_WRAPPER_EXPORTS)
#define PHYSX_GO_API __declspec(dllimport)
#else
#define PHYSX_GO_API
#endif
#else
#define PHYSX_GO_API  // Linux/macOS
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

	typedef void* PxGoFoundationHandle;
	typedef void* PxGoPhysicsHandle;
	typedef void* PxGoSceneHandle;
	typedef void* PxGoRigidDynamicHandle;
	typedef void* PxGoRigidStaticHandle;
	typedef void* PxGoMaterialHandle;
	typedef void* PxGoShapeHandle;
	typedef void* PxGoCookingHandle;
	typedef void* PxGoPvdHandle;
	typedef void* PxGoCollectionHandle;

	typedef struct {
		float x, y, z;
	} PxGoVec3;

	typedef struct {
		float x, y, z, w;
	} PxGoQuat;

	typedef struct {
		PxGoVec3 p;
		PxGoQuat q;
	} PxGoTransform;

	typedef struct {
		float radius;
	} PxGoSphereGeometry;

	typedef struct {
		PxGoVec3 halfExtents;
	} PxGoBoxGeometry;

	typedef struct {
		float radius;
		float halfHeight;
	} PxGoCapsuleGeometry;

	typedef struct {
		PxGoVec3 gravity;
		uint32_t maxActors;
		bool enableCCD;
	} PxGoSceneDesc;

	PHYSX_GO_API PxGoFoundationHandle PxGoCreateFoundation(uint32_t version, const char* allocatorName);
	PHYSX_GO_API void PxGoReleaseFoundation(PxGoFoundationHandle foundation);

	PHYSX_GO_API PxGoCollectionHandle PxGoLoadCollectionFromXmlFile(const char* path, PxGoPhysicsHandle physics, PxGoCookingHandle cookingHandle);// 从 XML 文件加载 Collection
	PHYSX_GO_API PxGoCollectionHandle PxGoLoadCollectionFromXmlMemory(const char* xmlData, size_t xmlSize, PxGoPhysicsHandle physics, PxGoCookingHandle cookingHandle); // 从内存 XML 字符串加载 Collection
	PHYSX_GO_API void PxGoReleaseCollection(PxGoCollectionHandle collection); // 释放 Collection
	PHYSX_GO_API PxGoRigidStaticHandle PxGoSceneCreateStaticActorFromCollection(PxGoSceneHandle sceneHandle, PxGoCollectionHandle collectionHandle, uint32_t index, PxGoTransform* transform);
	PHYSX_GO_API PxGoRigidDynamicHandle PxGoSceneCreateDynamicActorFromCollection(PxGoSceneHandle scene, PxGoCollectionHandle collection, uint32_t index, PxGoTransform* transform);
	PHYSX_GO_API PxGoRigidDynamicHandle PxGoSceneCreateKinematicActorFromCollection(PxGoSceneHandle sceneHandle, PxGoCollectionHandle collectionHandle, uint32_t index, PxGoTransform* transform);

	// 创建 PVD 对象
	PHYSX_GO_API PxGoPvdHandle PxGoCreatePvd(PxGoFoundationHandle foundation);
	PHYSX_GO_API bool PxGoConnectPvd(PxGoPvdHandle pvd, const char* host, int port);
	PHYSX_GO_API void PxGoReleasePvd(PxGoPvdHandle pvd);

	PHYSX_GO_API PxGoPhysicsHandle PxGoCreatePhysics(uint32_t version, PxGoFoundationHandle foundation, float toleranceScale, PxGoPvdHandle pvdHandle);
	PHYSX_GO_API void PxGoReleasePhysics(PxGoPhysicsHandle physics);

	PHYSX_GO_API PxGoCookingHandle PxGoCreateCooking(uint32_t version, PxGoFoundationHandle foundation);
	PHYSX_GO_API void PxGoReleaseCooking(PxGoCookingHandle cooking);

	PHYSX_GO_API PxGoSceneHandle PxGoCreateScene(PxGoPhysicsHandle physics, PxGoSceneDesc* desc);
	PHYSX_GO_API void PxGoReleaseScene(PxGoSceneHandle scene);
	PHYSX_GO_API void PxGoSceneSimulate(PxGoSceneHandle scene, float dt);
	PHYSX_GO_API bool PxGoSceneFetchResults(PxGoSceneHandle scene, bool block);
	PHYSX_GO_API void PxGoSceneAddActor(PxGoSceneHandle scene, PxGoRigidDynamicHandle actor);
	PHYSX_GO_API void PxGoSceneRemoveActor(PxGoSceneHandle scene, PxGoRigidDynamicHandle actor);
	PHYSX_GO_API void PxGoSceneAddStaticActor(PxGoSceneHandle scene, PxGoRigidStaticHandle actor);
	PHYSX_GO_API void PxGoSceneRemoveStaticActor(PxGoSceneHandle scene, PxGoRigidStaticHandle actor);

	PHYSX_GO_API PxGoMaterialHandle PxGoCreateMaterial(PxGoPhysicsHandle physics, float staticFriction,
		float dynamicFriction, float restitution);
	PHYSX_GO_API void PxGoReleaseMaterial(PxGoMaterialHandle material);

	PHYSX_GO_API PxGoShapeHandle PxGoCreateShapeSphere(PxGoPhysicsHandle physics, PxGoSphereGeometry* geometry,
		PxGoMaterialHandle material, bool isExclusive);
	PHYSX_GO_API PxGoShapeHandle PxGoCreateShapeBox(PxGoPhysicsHandle physics, PxGoBoxGeometry* geometry,
		PxGoMaterialHandle material, bool isExclusive);
	PHYSX_GO_API PxGoShapeHandle PxGoCreateShapeCapsule(PxGoPhysicsHandle physics, PxGoCapsuleGeometry* geometry,
		PxGoMaterialHandle material, bool isExclusive);
	PHYSX_GO_API void PxGoReleaseShape(PxGoShapeHandle shape);

	PHYSX_GO_API PxGoRigidDynamicHandle PxGoCreateRigidDynamic(PxGoPhysicsHandle physics, PxGoTransform* transform);
	PHYSX_GO_API void PxGoReleaseRigidDynamic(PxGoRigidDynamicHandle actor);
	PHYSX_GO_API void PxGoRigidDynamicAttachShape(PxGoRigidDynamicHandle actor, PxGoShapeHandle shape);
	PHYSX_GO_API void PxGoRigidDynamicSetMass(PxGoRigidDynamicHandle actor, float mass);
	PHYSX_GO_API void PxGoRigidDynamicSetLinearVelocity(PxGoRigidDynamicHandle actor, PxGoVec3* velocity);
	PHYSX_GO_API void PxGoRigidDynamicSetAngularVelocity(PxGoRigidDynamicHandle actor, PxGoVec3* velocity);
	PHYSX_GO_API void PxGoRigidDynamicGetGlobalPose(PxGoRigidDynamicHandle actor, PxGoTransform* transform);
	PHYSX_GO_API void PxGoRigidDynamicSetGlobalPose(PxGoRigidDynamicHandle actor, PxGoTransform* transform);
	PHYSX_GO_API void PxGoRigidDynamicAddForce(PxGoRigidDynamicHandle actor, PxGoVec3* force, uint32_t mode);
	PHYSX_GO_API void PxGoRigidDynamicGetLinearVelocity(PxGoRigidDynamicHandle actor, PxGoVec3* velocity);

	PHYSX_GO_API PxGoRigidStaticHandle PxGoCreateRigidStatic(PxGoPhysicsHandle physics, PxGoTransform* transform);
	PHYSX_GO_API void PxGoReleaseRigidStatic(PxGoRigidStaticHandle actor);
	PHYSX_GO_API void PxGoRigidStaticAttachShape(PxGoRigidStaticHandle actor, PxGoShapeHandle shape);
	PHYSX_GO_API void PxGoRigidStaticGetGlobalPose(PxGoRigidStaticHandle actor, PxGoTransform* transform);

	// 为 Kinematic Actor 设置目标位置（自动平滑移动）
	PHYSX_GO_API void PxGoRigidDynamicSetKinematicTarget(PxGoRigidDynamicHandle actor, PxGoTransform* target);

	PHYSX_GO_API PxGoVec3 PxGoVec3Make(float x, float y, float z);
	PHYSX_GO_API PxGoQuat PxGoQuatMake(float x, float y, float z, float w);
	PHYSX_GO_API PxGoQuat PxGoQuatIdentity();
	PHYSX_GO_API PxGoTransform PxGoTransformMake(PxGoVec3 position, PxGoQuat rotation);

#ifdef __cplusplus
}
#endif

#endif // PHYSX_C_API_H