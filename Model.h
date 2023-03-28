#pragma once

#include "RenderManager.h"






// 描画サブセットマテリアル構造体
struct SUBSET_MATERIAL
{
	char						Name[256];
	MATERIAL					Material;

	std::unique_ptr<TEXTURE>	TextureBaseColor;
	std::unique_ptr<TEXTURE>	TextureARM;

};

// 描画サブセット構造体
struct SUBSET
{
	char			Name[256];
	unsigned int	StartIndex;
	unsigned int	IndexNum;
	SUBSET_MATERIAL	Material;
};




// モデルサブセットマテリアル構造体
struct MODEL_SUBSET_MATERIAL
{
	char						Name[256];
	MATERIAL					Material;

	char						TextureNameBaseColor[256];
	char						TextureNameARM[256];
};



// モデルサブセット構造体
struct MODEL_SUBSET
{
	char					Name[256];
	unsigned int			StartIndex;
	unsigned int			IndexNum;
	MODEL_SUBSET_MATERIAL	Material;
};



// モデル構造体
struct MODEL
{
	VERTEX_3D		*VertexArray;
	unsigned int	VertexNum;

	unsigned int	*IndexArray;
	unsigned int	IndexNum;

	MODEL_SUBSET	*SubsetArray;
	unsigned int	SubsetNum;
};






class Model
{
private:

	std::unique_ptr<VERTEX_BUFFER>	m_VertexBuffer;
	std::unique_ptr<INDEX_BUFFER>	m_IndexBuffer;
	std::vector<SUBSET>				m_SubsetArray;


	void LoadObj( const char *FileName, MODEL *Model );
	void LoadMaterial( const char *FileName, MODEL_SUBSET_MATERIAL **MaterialArray, unsigned int *MaterialNum );

	void LoadObjBin(const char* FileName, MODEL* Model);
	void SaveObjBin(const char* FileName, MODEL* Model);

public:

	void Load(const char *FileName);
	void Draw(bool UseMaterial=true);



};