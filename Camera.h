#pragma once

class Camera
{
private:
	XMFLOAT3 m_Position{ 0.0f,0.0f,0.0f };
	XMFLOAT3 m_Target{ 0.0f,0.0f,0.0f };

	float       Exposure; //�I�o�␳
	float       Vignette; //�r�l�b�g
	float       Gamma;    //�K���}�␳
	float       Bloom;    //�u���[��
	float       Tone;     //�g�[���}�b�s���O
	float       Chromatic;//�F����
	float       Monokuro; //���m�N��

public:

	Camera();

	void Update();
	void Draw();


};

