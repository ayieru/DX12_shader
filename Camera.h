#pragma once

class Camera
{
private:
	XMFLOAT3 m_Position{ 0.0f,0.0f,0.0f };
	XMFLOAT3 m_Target{ 0.0f,0.0f,0.0f };

	float       Exposure; //露出補正
	float       Vignette; //ビネット
	float       Gamma;    //ガンマ補正
	float       Bloom;    //ブルーム
	float       Tone;     //トーンマッピング
	float       Chromatic;//色収差
	float       Monokuro; //モノクロ

public:

	Camera();

	void Update();
	void Draw();


};

