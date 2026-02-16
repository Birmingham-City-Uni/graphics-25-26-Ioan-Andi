#pragma once

#include <ostream>


// *** Your Code Here! ***
// * Task 1: I've started off a class here to store and manipulate 3D floating-point vectors.
//           We'll need to use this to load and draw the triangle mesh from the OBJ file.
//           Complete the functions indicated, so we can use the Vector3 class to draw our mesh!

std::string inputFilename = "../models/stanford_bunny_simplified.obj";

class Vector3
{
public:
	// This is the default constructor.
	// Change this to set the x, y and z components of the vector to all be zero.
	// You can use an initialiser list, or set them in the constructor function's body.

	Vector3()
	{
		x_ = 0.0f;
		y_ = 0.0f;
		z_ = 0.0f;
	}

	// This constructor can be used to set the x, y and z components of a vector when it
	// is created. Change this function to set the x_, y_ and z_ members to the values
	// provided.
	Vector3(float x, float y, float z)
	{
		x_ = x;
		y_ = y;
		z_ = z;
	}

	// Implement this method to add two vectors.
	Vector3 operator+(const Vector3& other) const
	{
		return Vector3(x_ + other.x_, y_ + other.y_, z_ + other.z_);
	}

	// Multiply the vector by a scalar.
	Vector3 operator*(float scalar) const
	{
		return Vector3(x_ * scalar, y_ * scalar, z_ * scalar);
	}

	// Get a component of the vector
	// This is the index operator - it's the one that is used when a user 
	// indexes part of a vector, for example:
	// myVec[0] = 1.0f; // Set the x component of this vector to 1.
	// Your code should return either x_, y_ or z_, depending on the value of i.
	// If the user tries to access an entry other than 0, 1 or 2, throw an
	// error.
	float& operator[](int i)
	{
		switch (i)
		{
		case 0: return x_;
		case 1: return y_;
		case 2: return z_;
		default:
			throw std::out_of_range("Vector3 index must be 0, 1, or 2");
		}
	}

	// This is the const version of the [] operator.
	// This version will be used when you have a const reference to a Vector3.
	// The code inside can be the same as for the non-const version above.
	float operator[](int i) const
	{
		// YOUR CODE HERE
	}

	// I have already written these getters for you.
	// Note that they return *references* to x_, y_ and z_
	// This means that they can be used to modify the values, for example:
	// Vector3 myVec;
	// myVec.x() = 10.0f;
	// The const versions return const references, which don't let you change
	// the value of x_, y_ or z_, so this would produce an error:
	// const Vector3& myVecReference;
	// myVecReference.x() = 10.0f;
	const float& x() const
	{
		return x_;
	}
	float& x()
	{
		return x_;
	}

	const float& y() const
	{
		return y_;
	}
	float& y()
	{
		return y_;
	}

	const float& z() const
	{
		return z_;
	}
	float& z()
	{
		return z_;
	}

private:
	// Add the data to store the x, y and z coordinates of the vector.
	float x_, y_, z_;
};

// Note: adding this version of operator* will let us do scalar * vector, as well as vector * scalar.
// see how it makes use of the operator* you define above.
Vector3 operator*(float scalar, const Vector3& vector)
{
	return vector * scalar;
}

// This tells us how to add the vector to a stream
// For example, how to print the vector with std::cout.
std::ostream& operator<<(std::ostream& stream, const Vector3& v)
{
	stream << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")";
	return stream;
}

