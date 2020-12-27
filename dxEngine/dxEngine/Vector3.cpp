#include "Vector3.h"
#include <cmath>


Vector3::Vector3():
	x(0),
	y(0),
	z(0){
	
}

Vector3::Vector3(float X, float Y, float Z):
	x(X),
	y(Y),
	z(Z){

}

float Vector3::length() const{
	return std::sqrtf(x * x + y * y + z * z);
}

Vector3 & Vector3::normalize(){
	if (length() == 0)
		return *this;
	else
		return *this /= length();
}

float Vector3::dot(const Vector3 & v) const{
	return x * v.x + y * v.y + z * v.z;
}

Vector3 Vector3::cross(const Vector3 & v) const{
	float X = y * v.z - z * v.y;
	float Y = z * v.x - x * v.z;
	float Z = x * v.y - y * v.x;
	return Vector3(X, Y, Z);
}

void Vector3::clear(double a)
{
	x = a;
	y = a;
	z = a;
}

Vector3 Vector3::operator+()const {
	return *this;
}

Vector3 Vector3::operator-()const {
	return Vector3(-x, -y, -z);
}

Vector3& Vector3::operator+=(const Vector3& v) {
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

Vector3& Vector3::operator-=(const Vector3& v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

Vector3& Vector3::operator*=(float s) {
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

Vector3& Vector3::operator/=(float s) {
	if (s == 0)
		return *this;
	else {
		x /= s;
		y /= s;
		z /= s;
		return *this;
	}
}

const Vector3 operator+(const Vector3& v1, const Vector3& v2) {
	Vector3 temp(v1);
	temp += v2;
	return temp;
}

const Vector3 operator-(const Vector3& v1, const Vector3& v2) {
	Vector3 temp(v1);
	temp -= v2;
	return temp;
}

const Vector3 operator*(const Vector3& v, float s) {
	Vector3 temp(v);
	temp *= s;
	return temp;
}

const Vector3 operator*(float s, const Vector3& v) {
	Vector3 temp(v);
	temp *= s;
	return temp;
}

const Vector3 operator/(const Vector3& v, float s) {
	Vector3 temp(v);
	temp /= s;
	return temp;
}
