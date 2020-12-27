#include"GameObject.h"

#include<string>

void GameObject::Collision(
	const GameObject& obj
){
}

void GameObject::DebugShader(
	const HRESULT& result
){
	if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
		::OutputDebugStringA("ƒtƒ@ƒCƒ‹‚ªŒ©“–‚½‚è‚Ü‚¹‚ñ");
		exit(0);
	}
	else {
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		::OutputDebugStringA(errstr.c_str());
	}
}
