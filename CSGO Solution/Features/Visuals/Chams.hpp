#pragma ocne
#include "../SDK/Includes.hpp"
#include "../Settings.hpp"

class C_Chams
{
public:
	virtual void CreateMaterials( );
	virtual void DrawModel( C_ChamsSettings Settings, IMatRenderContext* pCtx, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4_t* aMatrix, bool bForceNull = false, bool bXQZ = false );
	virtual void DrawPlayerChams(IMatRenderContext* pCtx, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4_t* aMatrix);
	virtual void OnModelRender( IMatRenderContext* pCtx, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4_t* aMatrix );
	virtual void ApplyMaterial( int32_t iMaterial, bool bIgnoreZ, Color aColor, bool bCustom = false, bool bShotChams = false, int32_t bWireFrame = 0 );
	virtual bool GetInterpolatedMatrix( C_BasePlayer* pPlayer, matrix3x4_t* aMatrix );
private:
	enum ModelType
	{
		MODELTYPE_INVALID,
		MODELTYPE_PLAYER,
		MODELTYPE_WEAPON,
		MODELTYPE_ARMS,
		MODELTYPE_VIEWWEAPON,
		MODELTYPE_BOMB,
		MODELTYPE_PROJ,
	};

	C_Material* m_pFlatMaterial = nullptr;
	C_Material* m_pGlowMaterial = nullptr;
	C_Material* m_pPulseMaterial = nullptr;
	C_Material* m_pRegularMaterial = nullptr;
	C_Material* m_pGlassMaterial = nullptr;
	C_Material* m_pGhostMaterial = nullptr;
	C_Material* m_pHaloMaterial = nullptr;

	ModelType m_iModelType = ModelType::MODELTYPE_INVALID;
};

inline C_Chams* g_Chams = new C_Chams( );