#pragma once

#ifndef LH_HISTOGRAM_H
#define LH_HISTOGRAM_H

#include <nvMath.h>
#include <vector>
#include <tuple>
#include <stack>

/// get a 1D index from a 3D index
unsigned int get_index(const int i, const int j, const int k, const int *sizes)
{
	return (i * sizes[1] + j) * sizes[0] + k;
}

int get_int(float value)
{
	return (int)value;
}

/// comparison functor of gradients
struct greater_than
{
	const int *_sizes;
	const std::vector<nv::vec3f> *_gradients;

	greater_than(const int *sizes, const std::vector<nv::vec3f> *gradients)
	{
		_sizes = sizes;
		_gradients = gradients;
	}

	nv::vec3f get_gradient(unsigned int index)
	{
		return (*_gradients)[index];
	}

	bool operator()(nv::vec3i v1, nv::vec3i v2)
	{
		auto i1 = get_index(v1.x, v1.y, v1.z, _sizes);
		auto i2 = get_index(v2.x, v2.y, v2.z, _sizes);
		auto g1 = get_gradient(i1);
		auto g2 = get_gradient(i2);
		return nv::square_norm(g1) > nv::square_norm(g2);
	}
};

/// find the minimum value along the gradient direction
nv::vec3i find_minima(const int *sizes, const std::vector<float> &scalar_value, const std::vector<nv::vec3f> &gradients, const int i, const int j, const int k, int sign, std::vector<nv::vec3i> &sub_trace)
{
	const float epsilon = 16;
	int boundary[3] = {sizes[0]-1, sizes[1]-1, sizes[2]-1};
	int width = sizes[0], height = sizes[1], depth = sizes[2];
	if (k==0 || j==0 || i==0 || k==boundary[0] || j==boundary[1] || i==boundary[2])
	{
		return nv::vec3i(i, j, k);
	}else
	{
		greater_than greater(sizes, &gradients);
		auto current_index = nv::vec3i(i, j, k);
		while (true)
		{
			sub_trace.push_back(current_index);
			auto i1 = current_index.x, j1 = current_index.y, k1 = current_index.z;
			auto index = get_index(i1, j1, k1, sizes);
			auto scalar0 = scalar_value[index];
			auto gradient0 = sign * gradients[index];
			if (nv::length(gradient0) > epsilon)
			{
				auto dx = nv::normalize(gradient0);
				auto pos = nv::vec3f(i1, j1, k1);
				while (get_int(pos.x) == i1 && get_int(pos.y) == j1 && get_int(pos.z) == k1)
				{
					pos += dx;
				}
				auto i0 = get_int(pos.x), j0 = get_int(pos.y), k0 = get_int(pos.z);
				if (k0>=0 && j0>=0 && i0>=0 && k0<=boundary[0] && j0<=boundary[1] && i0<=boundary[2])
				{
					auto new_index = nv::vec3i(i0, k0, j0);
					if (greater(new_index, current_index))
					{
						return current_index;
					} 
					else
					{
						current_index = new_index;
					}
				}else
				{
					return current_index;
				}
			} 
			else
			{
				return current_index;
			}
		}
	}
}

/// LH histogram
void calculate_LH_histogram(const int *sizes, const unsigned int count, const std::vector<float> &scalar_value, const std::vector<nv::vec3f> &gradients, std::vector<nv::vec2f> &lh_histogram, std::vector<std::vector<nv::vec3i>> &trace)
{
	int width = sizes[0], height = sizes[1], depth = sizes[2];

	for (int i=0; i<depth; i++)
	{
		for (int j=0; j<height; j++)
		{
			for (int k=0; k<width; k++)
			{
				auto index = get_index(i, j, k, sizes);
				auto lower = find_minima(sizes, scalar_value, gradients, i, j, k, -1, trace[index]);
				auto lower_index = get_index(lower.x, lower.y, lower.z, sizes);
				lh_histogram[index].x = scalar_value[lower_index];
				auto higher = find_minima(sizes, scalar_value, gradients, i, j, k, 1, trace[index]);
				auto higher_index = get_index(higher.x, higher.y, higher.z, sizes);
				lh_histogram[index].y = scalar_value[higher_index];
			}
		}
	}
}

#endif // LH_HISTOGRAM_H
