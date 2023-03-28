
#include <stdio.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "Main.h"
#include "RenderManager.h"
#include "Model.h"







void Model::Load(const char* FileName)
{
	RenderManager* renderManager = RenderManager::GetInstance();



	std::string dir(FileName);
	dir = dir.substr(0, dir.find_last_of("\\"));
	dir += "\\";


	std::string path(FileName);
	path = path.substr(0, path.find_last_of("."));
	path += ".objbin";

	


	//�t�@�C���ǂݍ���
	MODEL model{};
	{
		bool findBin = false;

		HANDLE handleBin = CreateFile(path.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (handleBin != INVALID_HANDLE_VALUE)
		{
			HANDLE handleObj = CreateFile(FileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (handleObj == INVALID_HANDLE_VALUE)
				assert(false);

			FILETIME timeBin, timeObj;
			GetFileTime(handleBin, NULL, NULL, &timeBin);
			GetFileTime(handleObj, NULL, NULL, &timeObj);

			if (CompareFileTime(&timeBin, &timeObj) >= 0)
				findBin = true;

			CloseHandle(handleObj);
		}

		CloseHandle(handleBin);



		if (findBin)
		//if (false)
		{
			LoadObjBin(path.c_str(), &model);
		}
		else
		{
			LoadObj(FileName, &model);
			SaveObjBin(path.c_str(), &model);
		}

	}





	//�o�b�t�@����
	{

		ComPtr<ID3D12Device> device = RenderManager::GetInstance()->GetDevice();



		//���_�o�b�t�@�̍쐬
		m_VertexBuffer = renderManager->CreateVertexBuffer(sizeof(VERTEX_3D), model.VertexNum);

		VERTEX_3D* vertex;
		m_VertexBuffer->Resource->Map(0, nullptr, (void**)&vertex);

		memcpy(vertex, model.VertexArray, sizeof(VERTEX_3D) * model.VertexNum);

		m_VertexBuffer->Resource->Unmap(0, nullptr);




		//�C���f�b�N�X�o�b�t�@�̍쐬
		m_IndexBuffer = renderManager->CreateIndexBuffer(model.IndexNum);

		unsigned int* index;
		m_IndexBuffer->Resource->Map(0, nullptr, (void**)&index);

		memcpy(index, model.IndexArray, sizeof(unsigned int) * model.IndexNum);

		m_IndexBuffer->Resource->Unmap(0, nullptr);
	}








	//�T�u�Z�b�g�ݒ�
	{
		m_SubsetArray.resize(model.SubsetNum);

		for (unsigned int i = 0; i < model.SubsetNum; i++)
		{
			strcpy(m_SubsetArray[i].Name, model.SubsetArray[i].Name);

			m_SubsetArray[i].StartIndex = model.SubsetArray[i].StartIndex;
			m_SubsetArray[i].IndexNum = model.SubsetArray[i].IndexNum;
			m_SubsetArray[i].Material.Material = model.SubsetArray[i].Material.Material;

			strcpy(m_SubsetArray[i].Material.Name, model.SubsetArray[i].Material.Name);

			if (strlen(model.SubsetArray[i].Material.TextureNameBaseColor) != 0)
				m_SubsetArray[i].Material.TextureBaseColor = RenderManager::GetInstance()->LoadTexture((dir + model.SubsetArray[i].Material.TextureNameBaseColor).c_str());

			if (strlen(model.SubsetArray[i].Material.TextureNameARM) != 0)
				m_SubsetArray[i].Material.TextureARM = RenderManager::GetInstance()->LoadTexture((dir + model.SubsetArray[i].Material.TextureNameARM).c_str());
		}
	}





	//�A���t�@�l�Ń\�[�g
	std::sort(
		m_SubsetArray.begin(),
		m_SubsetArray.end(),
		[](const SUBSET& subset1, const SUBSET& subset2)
		{
			return subset1.Material.Material.BaseColor.w > subset2.Material.Material.BaseColor.w;
		}
	);





	assert(model.VertexArray);
	assert(model.IndexArray);
	assert(model.SubsetArray);

	delete[] model.VertexArray;
	delete[] model.IndexArray;
	delete[] model.SubsetArray;


}





void Model::Draw(bool UseMaterial)
{

	RenderManager* renderManager = RenderManager::GetInstance();
	ID3D12GraphicsCommandList* CommandList = renderManager->GetGraphicsCommandList();


	//���_�o�b�t�@�ݒ�
	renderManager->SetVertexBuffer(m_VertexBuffer.get());

	//�C���f�b�N�X�o�b�t�@�ݒ�
	renderManager->SetIndexBuffer(m_IndexBuffer.get());

	//�g�|���W�ݒ�
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



	for (SUBSET& subset: m_SubsetArray)
	{
		if (UseMaterial)
		{
			renderManager->SetConstant(RenderManager::CONSTANT_TYPE::SUBSET, &subset.Material.Material, sizeof(MATERIAL));

			if (subset.Material.TextureBaseColor.get() != nullptr)
				renderManager->SetTexture(RenderManager::TEXTURE_TYPE::BASE_COLOR, subset.Material.TextureBaseColor.get());

			if (subset.Material.TextureARM.get() != nullptr)
				renderManager->SetTexture(RenderManager::TEXTURE_TYPE::ARM, subset.Material.TextureARM.get());
		}

		CommandList->DrawIndexedInstanced(subset.IndexNum, 1, subset.StartIndex, 0, 0);
	}

}





void Model::LoadObjBin(const char* FileName, MODEL* Model)
{
	FILE* file;
	file = fopen(FileName, "rb");
	assert(file);



	fread(&Model->VertexNum, sizeof(Model->VertexNum), 1, file);

	Model->VertexArray = new VERTEX_3D[Model->VertexNum];

	fread(Model->VertexArray, sizeof(VERTEX_3D), Model->VertexNum, file);




	fread(&Model->IndexNum, sizeof(Model->IndexNum), 1, file);

	Model->IndexArray = new unsigned int[Model->IndexNum];

	fread(Model->IndexArray, sizeof(unsigned int), Model->IndexNum, file);




	fread(&Model->SubsetNum, sizeof(Model->SubsetNum), 1, file);

	Model->SubsetArray = new MODEL_SUBSET[Model->SubsetNum];

	fread(Model->SubsetArray, sizeof(MODEL_SUBSET), Model->SubsetNum, file);



	fclose(file);

}



void Model::SaveObjBin(const char* FileName, MODEL* Model)
{
	FILE* file;
	file = fopen(FileName, "wb");
	assert(file);



	fwrite(&Model->VertexNum, sizeof(Model->VertexNum), 1, file);
	fwrite(Model->VertexArray, sizeof(VERTEX_3D), Model->VertexNum, file);


	fwrite(&Model->IndexNum, sizeof(Model->IndexNum), 1, file);
	fwrite(Model->IndexArray, sizeof(unsigned int), Model->IndexNum, file);


	fwrite(&Model->SubsetNum, sizeof(Model->SubsetNum), 1, file);
	fwrite(Model->SubsetArray, sizeof(MODEL_SUBSET), Model->SubsetNum, file);



	fclose(file);

}






void Model::LoadObj( const char *FileName, MODEL *Model )
{

	std::string dir(FileName);
	dir = dir.substr(0, dir.find_last_of("\\"));



	XMFLOAT3		*positionArray = nullptr;
	XMFLOAT3		*normalArray = nullptr;
	XMFLOAT2		*texcoordArray = nullptr;
	XMFLOAT4		*colorArray = nullptr;

	unsigned int	positionNum = 0;
	unsigned int	normalNum = 0;
	unsigned int	texcoordNum = 0;
	unsigned int	colorNum = 0;

	unsigned int	vertexNum = 0;
	unsigned int	indexNum = 0;
	unsigned int	in = 0;
	unsigned int	subsetNum = 0;

	MODEL_SUBSET_MATERIAL	*materialArray = nullptr;
	unsigned int	materialNum = 0;

	char str[256];
	char *s;
	char c;


	FILE *file;
	file = fopen( FileName, "rt" );
	assert(file);



	//�v�f���J�E���g
	while( true )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;

		if( strcmp( str, "v" ) == 0 )
		{
			positionNum++;
		}
		else if( strcmp( str, "vn" ) == 0 )
		{
			normalNum++;
		}
		else if (strcmp(str, "vc") == 0)
		{
			colorNum++;
		}
		else if( strcmp( str, "vt" ) == 0 )
		{
			texcoordNum++;
		}
		else if( strcmp( str, "usemtl" ) == 0 )
		{
			subsetNum++;
		}
		else if( strcmp( str, "f" ) == 0 )
		{
			in = 0;

			do
			{
				fscanf( file, "%s", str );
				vertexNum++;
				in++;
				c = fgetc( file );
			}
			while( c != '\n' && c!= '\r' );

			//�l�p�͎O�p�ɕ���
			if( in == 4 )
				in = 6;

			indexNum += in;
		}
	}


	//�������m��
	positionArray = new XMFLOAT3[ positionNum ];
	normalArray = new XMFLOAT3[ normalNum ];
	texcoordArray = new XMFLOAT2[ texcoordNum ];
	colorArray = new XMFLOAT4[ colorNum ];


	Model->VertexArray = new VERTEX_3D[ vertexNum ];
	Model->VertexNum = vertexNum;

	Model->IndexArray = new unsigned int[ indexNum ];
	Model->IndexNum = indexNum;

	Model->SubsetArray = new MODEL_SUBSET[ subsetNum ];
	Model->SubsetNum = subsetNum;




	//�v�f�Ǎ�
	XMFLOAT3* position = positionArray;
	XMFLOAT3* normal = normalArray;
	XMFLOAT2* texcoord = texcoordArray;
	XMFLOAT4* color = colorArray;

	unsigned int vc = 0;
	unsigned int ic = 0;
	unsigned int sc = 0;

	char objectName[256];


	fseek( file, 0, SEEK_SET );

	while( true )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;

		if( strcmp( str, "mtllib" ) == 0 )
		{
			//�}�e���A���t�@�C��
			fscanf( file, "%s", str );

			char path[256];
			strcpy( path, dir.c_str() );
			strcat( path, "\\" );
			strcat( path, str );

			LoadMaterial( path, &materialArray, &materialNum );
		}
		else if( strcmp( str, "o" ) == 0 )
		{
			//�I�u�W�F�N�g��
			fscanf( file, "%s", objectName);
		}
		else if( strcmp( str, "v" ) == 0 )
		{
			//���_���W
			fscanf( file, "%f", &position->x );
			fscanf( file, "%f", &position->y );
			fscanf( file, "%f", &position->z );
			position->x *= -1.0f;
			//*position *= 100.0f;
			position++;
		}
		else if( strcmp( str, "vn" ) == 0 )
		{
			//�@��
			fscanf( file, "%f", &normal->x );
			fscanf( file, "%f", &normal->y );
			fscanf( file, "%f", &normal->z );
			normal->x *= -1.0f;
			normal++;
		}
		else if (strcmp(str, "vc") == 0)
		{
			//���_�F
			fscanf(file, "%f", &color->x);
			fscanf(file, "%f", &color->y);
			fscanf(file, "%f", &color->z);
			fscanf(file, "%f", &color->w);
			color++;
		}

		else if( strcmp( str, "vt" ) == 0 )
		{
			//�e�N�X�`�����W
			fscanf( file, "%f", &texcoord->x );
			fscanf( file, "%f", &texcoord->y );
			texcoord->y = 1.0f - texcoord->y;
			texcoord++;
		}
		else if( strcmp( str, "usemtl" ) == 0 )
		{
			//�}�e���A��
			fscanf( file, "%s", str );

			strcpy(Model->SubsetArray[sc].Name, objectName);

			if( sc != 0 )
				Model->SubsetArray[ sc - 1 ].IndexNum = ic - Model->SubsetArray[ sc - 1 ].StartIndex;

			Model->SubsetArray[ sc ].StartIndex = ic;


			for( unsigned int i = 0; i < materialNum; i++ )
			{
				if( strcmp( str, materialArray[i].Name ) == 0 )
				{
					Model->SubsetArray[ sc ].Material = materialArray[i];
					break;
				}
			}

			sc++;
			
		}
		else if( strcmp( str, "f" ) == 0 )
		{
			//��
			in = 0;

			do
			{
				fscanf( file, "%s", str );

				s = strtok( str, "/" );	
				Model->VertexArray[vc].Position = positionArray[ atoi( s ) - 1 ];

				if( s[ strlen( s ) + 1 ] != '/' )
				{
					//�e�N�X�`�����W�����݂��Ȃ��ꍇ������
					s = strtok( NULL, "/" );
					Model->VertexArray[vc].TexCoord = texcoordArray[ atoi( s ) - 1 ];
				}

				s = strtok( NULL, "/" );	
				Model->VertexArray[vc].Normal = normalArray[ atoi( s ) - 1 ];

				s = strtok( NULL, "/" );
				if (s)
					Model->VertexArray[vc].Color = colorArray[atoi(s) - 1];
				else
					Model->VertexArray[vc].Color = { 1.0f, 1.0f, 1.0f, 1.0f };


				Model->IndexArray[ic] = vc;
				ic++;
				vc++;

				in++;
				c = fgetc( file );
			}
			while( c != '\n' && c != '\r' );

			std::swap(Model->IndexArray[ic - in], Model->IndexArray[ic - in + 1]);

			//�l�p�͎O�p�ɕ���
			if( in == 4 )
			{
				Model->IndexArray[ic] = vc - 2;
				ic++;
				Model->IndexArray[ic] = vc - 4;
				ic++;
			}
		}
	}


	if( sc != 0 )
		Model->SubsetArray[ sc - 1 ].IndexNum = ic - Model->SubsetArray[ sc - 1 ].StartIndex;


	fclose(file);


	assert(positionArray);
	assert(normalArray);
	assert(texcoordArray);
	assert(colorArray);

	delete[] positionArray;
	delete[] normalArray;
	delete[] texcoordArray;
	delete[] colorArray;



	assert(materialArray);

	delete[] materialArray;
}






void Model::LoadMaterial( const char *FileName, MODEL_SUBSET_MATERIAL **MaterialArray, unsigned int *MaterialNum )
{

	char str[256];

	FILE *file;
	file = fopen( FileName, "rt" );
	assert(file);

	MODEL_SUBSET_MATERIAL* materialArray;
	unsigned int materialNum = 0;

	//�v�f���J�E���g
	while( true )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;


		if( strcmp( str, "newmtl" ) == 0 )
		{
			materialNum++;
		}
	}


	//�������m��
	materialArray = new MODEL_SUBSET_MATERIAL[ materialNum ];


	//�v�f�Ǎ�
	int mc = -1;

	fseek( file, 0, SEEK_SET );

	while( true )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;


		if( strcmp( str, "newmtl" ) == 0 )
		{
			//�}�e���A����
			mc++;
			fscanf( file, "%s", materialArray[ mc ].Name );

			strcpy(materialArray[mc].TextureNameBaseColor, "");
			strcpy(materialArray[mc].TextureNameARM, "");
		}
		else if( strcmp( str, "Ka" ) == 0 )
		{
			//�A���r�G���g
			float ambient;
			fscanf(file, "%f", &ambient);
			fscanf(file, "%f", &ambient);
			fscanf(file, "%f", &ambient);
		}
		else if( strcmp( str, "Kd" ) == 0 )
		{
			//�f�B�t���[�Y
			fscanf( file, "%f", &materialArray[ mc ].Material.BaseColor.x );
			fscanf( file, "%f", &materialArray[ mc ].Material.BaseColor.y );
			fscanf( file, "%f", &materialArray[ mc ].Material.BaseColor.z );
			materialArray[ mc ].Material.BaseColor.w = 1.0f;
		}
		else if (strcmp(str, "Ke") == 0)
		{
			//�G�~�b�V����
			fscanf(file, "%f", &materialArray[mc].Material.EmissionColor.x);
			fscanf(file, "%f", &materialArray[mc].Material.EmissionColor.y);
			fscanf(file, "%f", &materialArray[mc].Material.EmissionColor.z);
			materialArray[mc].Material.EmissionColor.w = 1.0f;
		}
		else if( strcmp( str, "Ks" ) == 0 )
		{
			//�X�y�L����
			float specular;
			fscanf( file, "%f", &specular );
			fscanf( file, "%f", &specular );
			fscanf( file, "%f", &specular );

			materialArray[mc].Material.Specular = specular;
		}
		else if( strcmp( str, "Ns" ) == 0 )
		{
			//�X�y�L�������x
			float shininess;
			fscanf( file, "%f", &shininess);
		}
		else if( strcmp( str, "d" ) == 0 )
		{
			//�A���t�@
			fscanf( file, "%f", &materialArray[ mc ].Material.BaseColor.w );
		}
		else if (strcmp(str, "Metallic") == 0)
		{
			//���^���b�N
			fscanf(file, "%f", &materialArray[mc].Material.Metallic);
		}
		else if (strcmp(str, "Roughness") == 0)
		{
			//���t�l�X
			fscanf(file, "%f", &materialArray[mc].Material.Roughness);
		}
		else if( strcmp( str, "map_Kd" ) == 0 )
		{
			//�e�N�X�`��
			fscanf( file, "%s", str );
			strcpy( materialArray[ mc ].TextureNameBaseColor, str);
		}
		else if (strcmp(str, "map_ARM") == 0)
		{
			//ARM�e�N�X�`��
			fscanf(file, "%s", str);
			strcpy(materialArray[mc].TextureNameARM, str);
		}
	}

	fclose(file);


	*MaterialArray = materialArray;
	*MaterialNum = materialNum;

}

