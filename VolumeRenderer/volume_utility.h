/**	@file
*	a header file for volume data manipulation
*/

#ifndef volume_utility_h
#define volume_utility_h

#include <fstream>
#include <algorithm>
//#include <functional>      // For greater<int>()
using namespace std;

//#include "K_Means.h"
//#include "K_Means_PP.h"
//#include "K_Means_Local.h"
//#include "K_Means_PlusPlus.h"
#include "K_Means_PP_Generic.h"
//#include "Fuzzy_CMeans.h"

/**	@brief	Classes and functions for volume manipulation
*	
*/
namespace volume_utility
{
	//#ifdef _DEBUG
	//#define _DEBUG_OUTPUT
	//#endif

	/// convert a char to a hex number, the cluster number is represented in 0~9 and a~f
	unsigned char char_to_number(unsigned char c)
	{
		c = tolower(c);
		if (c >= '0' && c <= '9')
		{
			return c - '0';
		}
		if (c >= 'a' && c <= 'f')
		{
			return 10 + c - 'a';
		}
		return c;
	}

	/// estimate the interval among clusters
	/// interval = (2^int(log(2, 256/k)))/256
	float get_cluster_interval( int k ) 
	{
		int shift = static_cast<int>(std::log(256./k)/std::log(2.));
		return (1 << shift) / 256.;
	}

	/// The bandwagon effect filter (The term is my invention ^_^)
	/// For each member, if more than a half of my neighbors belong to a group, I will join the group too
	void bandwagon_effect_filter(const int k_for_k_means, const unsigned char * label_ptr_before, unsigned char *& label_ptr_after, const int width, const int height, const int depth)
	{
		const int N = 7;
		const int dx[N] = {0, -1, 1, 0, 0, 0, 0};
		const int dy[N] = {0, 0, 0, -1, 1, 0, 0};
		const int dz[N] = {0, 0, 0, 0, 0, -1, 1};
		int *counter = new int[k_for_k_means];

#ifdef _DEBUG_OUTPUT
		ofstream f("d:\\Bandwagon_effect_filter_before.txt", ios::out);
		ofstream f2("d:\\Bandwagon_effect_filter_after.txt", ios::out);
		ofstream changes_log("d:\\Bandwagon_effect_filter_changes_log.txt", ios::out);
#endif

		for (int i=0; i<depth; i++)
		{
			for (int j=0; j<height; j++)
			{
				for (int k=0; k<width; k++)
				{
					int index = (i * height + j) * width + k;
					if (k==0 || k==width-1 || j==0 || j==height-1 || i==0 || i==depth-1)
					{
						label_ptr_after[index] = label_ptr_before[index];
					}else
					{
						int label_max = -1;
						memset(counter, 0, sizeof(int) * k_for_k_means);

						// the bandwagon effect filter
						for (int ii=0; ii<N; ii++)
						{
							int label = label_ptr_before[((i + dx[ii]) * height + j + dy[ii]) * width + k + dz[ii]]; 
							counter[label]++;
							if (label_max == -1)
							{
								label_max = label;
							}else
							{
								if (counter[label] > counter[label_max])
								{
									label_max = label;
								}
							}
						}
						label_ptr_after[index] = (counter[label_max] >= N / 2) ? label_max : label_ptr_before[index];

#ifdef _DEBUG_OUTPUT
						if (label_ptr_before[index] != label_ptr_after[index])
						{
							changes_log<<"i,j,k="<<i<<","<<j<<","<<k<<"\tindex="<<index<<std::endl;
							changes_log<<"counter:\t";
							for (int ii=0; ii<k_for_k_means; ii++)
							{
								changes_log<<counter[ii]<<" ";
							}
							changes_log<<endl;
							changes_log<<"label_ptr_before[index]="<<(int)label_ptr_before[index]<<"\tlabel_ptr_after[index]="<<(int)label_ptr_after[index]<<std::endl;						}
#endif

					}

#ifdef _DEBUG_OUTPUT
					f<<ios::hex<<(int)label_ptr_before[index]<<" ";
					f2<<ios::hex<<(int)label_ptr_after[index]<<" ";
#endif

				}
			}
		}
		delete[] counter;
	}

	/// use a median filter on scalar values
	void median_filter(const vector<float> &scalar_value_before, vector<float> &scalar_value, const int width, const int height, const int depth) 
	{
		const int N = 7;
		const int dx[N] = {0, -1, 1, 0, 0, 0, 0};
		const int dy[N] = {0, 0, 0, -1, 1, 0, 0};
		const int dz[N] = {0, 0, 0, 0, 0, -1, 1};
		vector<float> sorting(7);

#ifdef _DEBUG_OUTPUT
		ofstream f("d:\\median_filter_before.txt", ios::out);
		ofstream f2("d:\\median_filter_after.txt", ios::out);
#endif

		for (int i=0; i<depth; i++)
		{
			for (int j=0; j<height; j++)
			{
				for (int k=0; k<width; k++)
				{
					int index = (i * height + j) * width + k;
					if (k==0 || k==width-1 || j==0 || j==height-1 || i==0 || i==depth-1)
					{
						scalar_value[index] = scalar_value_before[index];
					}else
					{
						// median filter
						for (int ii=0; ii<N; ii++)
						{
							sorting[ii] = scalar_value_before[((i + dx[ii]) * height + j + dy[ii]) * width + k + dz[ii]];
						}
						partial_sort(sorting.begin(), sorting.begin() + (N + 1) / 2, sorting.end());
						scalar_value[index] = sorting[N / 2];
					}

#ifdef _DEBUG_OUTPUT
					f<<ios::hex<<scalar_value_before[index]<<" ";
					f2<<ios::hex<<scalar_value[index]<<" ";
#endif

				}
			}
		}
	}

	/// shift the cluster labels into the range of 0 to 255
	void shift_labels(const int k, const unsigned int count, unsigned char *& label_ptr)
	{
		int shift = static_cast<int>(std::log(256.0 / k) / std::log(2.0));

#ifdef _DEBUG_OUTPUT
		std::cout<<"label shift="<<shift<<std::endl;
		ofstream f("d:\\K_Means_debug_output.txt", ios::out);
		for (unsigned int i=0; i<count; i++)
		{
			f<<static_cast<int>(label_ptr[i]);
		}
		f<<endl;
#endif

		for (unsigned int i=0; i<count; i++)
		{
			label_ptr[i] = label_ptr[i] << shift;
		}
	}

	/// calculate the gradient and derivatives and do clustering on voxels
	template <class T, int TYPE_SIZE>
	void cluster(const T *data, const unsigned int count, const unsigned int components, const int k, unsigned char *& label_ptr, int width, int height, int depth)
	{
		unsigned int histogram[TYPE_SIZE] = {0};
		vector<float> scalar_value(count); // the scalar data in const T *data
		vector<nv::vec3f> gradient(count);
		vector<float> gradient_magnitude(count);
		vector<nv::vec3f> second_derivative(count);
		vector<float> second_derivative_magnitude(count);
		float max_gradient_magnitude, max_second_derivative_magnitude;

		// median filter
		//vector<float> scalar_value_before(count);
		//generate_scalar_histogram<T, TYPE_SIZE>(data, count, components, histogram, scalar_value_before);
		//median_filter(scalar_value_before, scalar_value, width, height, depth);

		std::cout<<"Scalar histogram..."<<std::endl;
		generate_scalar_histogram<T, TYPE_SIZE>(data, count, components, histogram, scalar_value);

		std::cout<<"Gradients and second derivatives..."<<std::endl;
		generate_gradient(sizes, scalar_value, gradient, gradient_magnitude, max_gradient_magnitude);
		generate_second_derivative(sizes, gradient, second_derivative, second_derivative_magnitude, max_second_derivative_magnitude);

		unsigned char *label_ptr_before_filter = new unsigned char[count];

		//// by Ben for statistical based clustering
		//vector<float> average(count);
		//vector<float> variation(count);
		//generate_average_variation(sizes, count, components, scalar_value, average, variation);

		// wrap for K_Means_PP_Generic::k_means()
		//std::vector<nv::vec2f> v(count);
		std::vector<nv::vec3f> v(count);
		//std::vector<nv::vec4f> v(count);
		for (unsigned int i=0; i<count; i++)
		{
			//v[i].x = average[i];
			//v[i].y = variation[i];

			v[i].x = scalar_value[i] / (float)TYPE_SIZE;
			v[i].y = gradient_magnitude[i] / max_gradient_magnitude;
			v[i].z = second_derivative_magnitude[i] / max_second_derivative_magnitude;

			//v[i].x = gradient[i].x;
			//v[i].y = gradient[i].y;
			//v[i].z = gradient[i].z;
			//v[i].w = scalar_value[i];
		}

		// call the clustering routine
		std::cout<<"Clustering..."<<std::endl;

		//clustering::K_Means_PP_DIY::k_means(count, scalar_value, gradient_magnitude, second_derivative_magnitude, k, label_ptr_before);

		//// by Ben for statistical based clustering
		//clustering::K_Means_PP_Generic::k_means(v, k, label_ptr_before_filter, clustering::K_Means_PP_Generic::get_distance<nv::vec2f>, clustering::K_Means_PP_Generic::get_centroid<nv::vec2f>);

		clustering::K_Means_PP_Generic::k_means(v, k, label_ptr_before_filter, clustering::K_Means_PP_Generic::get_distance<nv::vec3f>, clustering::K_Means_PP_Generic::get_centroid<nv::vec3f>);
		//clustering::K_Means_PP_Generic::k_means(v, k, label_ptr_before_filter, clustering::K_Means_PP_Generic::get_distance<nv::vec4f>, clustering::K_Means_PP_Generic::get_centroid<nv::vec4f>);

		//clustering::Fuzzy_CMeans::k_means(v, k, label_ptr_before_filter);

		//std::ofstream label_file_before("d:/label_before.txt");
		//for (unsigned int i=0; i<count; i++)
		//{
		//	label_file_before<<(int)label_ptr_before[i];
		//}

		// the bandwagon effect filter
		std::cout<<"Filtering..."<<std::endl;
		bandwagon_effect_filter(k, label_ptr_before_filter, label_ptr, width, height, depth);
		delete[] label_ptr_before_filter;

		//std::ofstream label_file("d:/label.txt");
		//for (unsigned int i=0; i<count; i++)
		//{
		//	label_file<<(int)label_ptr[i];
		//}

		//std::cout<<"Shifting labels..."<<std::endl;
		//shift_labels(k, count, label_ptr);
		//std::cout<<"The k_means routine is done."<<std::endl<<std::endl;
	}

	/// calculate scalar histogram
	template <class T, int TYPE_SIZE>
	void generate_scalar_histogram(const T *data, const unsigned int count, const unsigned int components, unsigned int *histogram, vector<float> &scalar_value)
	{
		for (unsigned int i=0; i<count; i++)
		{
			unsigned int temp = 0;
			unsigned int index = i * components;
			for (unsigned int j=0; j<components; j++)
			{
				temp += data[index + j];
			}
			temp = temp / components;
			histogram[temp]++;
			scalar_value[i] = temp;
		}
	}

	/// find the min and max scalar value for histogram equalization in shaders
	template <class T, int TYPE_SIZE>
	void find_min_max_scalar_in_histogram(const unsigned int count, const unsigned int *histogram, float &scalar_min, float &scalar_max)
	{
		const unsigned int min_amount = static_cast<unsigned int>(count * 0.023);
		unsigned int amount = min_amount;
		unsigned int min_index = 0;
		for (unsigned int i=0; i<TYPE_SIZE; i++)
		{
			if (histogram[i] >= amount)
			{
				min_index = i;
				break;
			}else
			{
				amount -= histogram[i];
			}
		}
		amount = min_amount;
		unsigned int max_index = TYPE_SIZE - 1;
		for (unsigned int i=TYPE_SIZE-1; i>=0; i--)
		{
			if (histogram[i] >= amount)
			{
				max_index = i;
				break;
			}else
			{
				amount -= histogram[i];
			}
		}
		scalar_min = static_cast<float>(min_index) / TYPE_SIZE;
		scalar_max = static_cast<float>(max_index) / TYPE_SIZE;

#ifdef _DEBUG_OUTPUT
		std::cout<<"min max index in the histogram\t"<<min_index<<"\t"<<max_index<<endl
			<<"min max float in the histogram\t"<<scalar_min<<"\t"<<scalar_max<<endl;
#endif
	}

	/// get a 1D index from a 3D index
	unsigned int get_index(const int i, const int j, const int k, const int *sizes)
	{
		return (i * sizes[1] + j) * sizes[0] + k;
	}

	/// Sobel operator
	nv::vec3f sobel(int i, int j, int k, const vector<float> &scalar_value, const int *sizes)
	{
		nv::vec3f gradient;
		gradient.x
			= 4.0 * (scalar_value[get_index(i+1, j, k, sizes)] - scalar_value[get_index(i-1, j, k, sizes)])
			+ 2.0 * (scalar_value[get_index(i+1, j-1, k, sizes)] - scalar_value[get_index(i-1, j-1, k, sizes)])
			+ 2.0 * (scalar_value[get_index(i+1, j+1, k, sizes)] - scalar_value[get_index(i-1, j+1, k, sizes)])
			+ 2.0 * (scalar_value[get_index(i+1, j, k-1, sizes)] - scalar_value[get_index(i-1, j, k-1, sizes)])
			+ 2.0 * (scalar_value[get_index(i+1, j, k+1, sizes)] - scalar_value[get_index(i-1, j, k+1, sizes)])
			+ (scalar_value[get_index(i+1, j-1, k-1, sizes)] - scalar_value[get_index(i-1, j-1, k-1, sizes)])
			+ (scalar_value[get_index(i+1, j+1, k+1, sizes)] - scalar_value[get_index(i-1, j+1, k+1, sizes)])
			+ (scalar_value[get_index(i+1, j+1, k-1, sizes)] - scalar_value[get_index(i-1, j+1, k-1, sizes)])
			+ (scalar_value[get_index(i+1, j-1, k+1, sizes)] - scalar_value[get_index(i-1, j-1, k+1, sizes)]);

		gradient.y
			= 4.0 * (scalar_value[get_index(i, j+1, k, sizes)] - scalar_value[get_index(i, j-1, k, sizes)])
			+ 2.0 * (scalar_value[get_index(i-1, j+1, k, sizes)] - scalar_value[get_index(i-1, j-1, k, sizes)])
			+ 2.0 * (scalar_value[get_index(i+1, j+1, k, sizes)] - scalar_value[get_index(1+1, j-1, k, sizes)])
			+ 2.0 * (scalar_value[get_index(i, j+1, k-1, sizes)] - scalar_value[get_index(i, j-1, k-1, sizes)])
			+ 2.0 * (scalar_value[get_index(i, j+1, k+1, sizes)] - scalar_value[get_index(i, j-1, k+1, sizes)])
			+ (scalar_value[get_index(i-1, j+1, k-1, sizes)] - scalar_value[get_index(i-1, j-1, k-1, sizes)])
			+ (scalar_value[get_index(i+1, j+1, k+1, sizes)] - scalar_value[get_index(1+1, j-1, k+1, sizes)])
			+ (scalar_value[get_index(i+1, j+1, k-1, sizes)] - scalar_value[get_index(i+1, j-1, k-1, sizes)])
			+ (scalar_value[get_index(i-1, j+1, k+1, sizes)] - scalar_value[get_index(i-1, j-1, k+1, sizes)]);

		gradient.z
			= 4.0 * (scalar_value[get_index(i, j, k+1, sizes)] - scalar_value[get_index(i, j, k-1, sizes)])
			+ 2.0 * (scalar_value[get_index(i-1, j, k+1, sizes)] - scalar_value[get_index(i-1, j, k-1, sizes)])
			+ 2.0 * (scalar_value[get_index(i+1, j, k+1, sizes)] - scalar_value[get_index(i+1, j, k-1, sizes)])
			+ 2.0 * (scalar_value[get_index(i, j-1, k+1, sizes)] - scalar_value[get_index(i, j-1, k-1, sizes)])
			+ 2.0 * (scalar_value[get_index(i, j+1, k+1, sizes)] - scalar_value[get_index(i, j+1, k-1, sizes)])
			+ (scalar_value[get_index(i-1, j-1, k+1, sizes)] - scalar_value[get_index(i-1, j-1, k-1, sizes)])
			+ (scalar_value[get_index(i+1, j+1, k+1, sizes)] - scalar_value[get_index(i+1, j+1, k-1, sizes)])
			+ (scalar_value[get_index(i+1, j-1, k+1, sizes)] - scalar_value[get_index(i+1, j-1, k-1, sizes)])
			+ (scalar_value[get_index(i-1, j+1, k+1, sizes)] - scalar_value[get_index(i-1, j+1, k-1, sizes)]);

		return gradient / 32;
	}

	/// calculate gradients
	void generate_gradient(const int *sizes, const vector<float> &scalar_value, vector<nv::vec3f> &gradient, vector<float> &gradient_magnitude, float &max_gradient_magnitude)
	{
		int boundary[3] = {sizes[0]-1, sizes[1]-1, sizes[2]-1};
		int width = sizes[0], height = sizes[1], depth = sizes[2];

		max_gradient_magnitude = -1;
		for (int i=0; i<depth; i++)
		{
			for (int j=0; j<height; j++)
			{
				for (int k=0; k<width; k++)
				{
					auto index = ((i) * height + j) * width + k;
					if (k==0 || j==0 || i==0 || k==boundary[0] || j==boundary[1] || i==boundary[2])
					{
						gradient_magnitude[index] = gradient[index].x = gradient[index].y = gradient[index].z = 0;
					}else
					{
						//gradient[index].z = scalar_value[((i + 1) * height + j) * width + k] - scalar_value[((i - 1) * height + j) * width + k];
						//gradient[index].y = scalar_value[((i) * height + j + 1) * width + k] - scalar_value[((i) * height + j - 1) * width + k];
						//gradient[index].x = scalar_value[((i) * height + j) * width + k + 1] - scalar_value[((i) * height + j) * width + k - 1];
						gradient[index] = sobel(i, j, k, scalar_value, sizes);
						gradient_magnitude[index] = length(gradient[index]);
						max_gradient_magnitude = std::max(gradient_magnitude[index], max_gradient_magnitude);
					}
				}
			}
		}
	}

	/// calculate second order derivatives
	void generate_second_derivative(const int *sizes, const vector<nv::vec3f> &gradient, vector<nv::vec3f> &second_derivative, vector<float> &second_derivative_magnitude, float &max_second_derivative_magnitude)
	{
		int boundary[3] = {sizes[0]-1, sizes[1]-1, sizes[2]-1};
		int width = sizes[0], height = sizes[1], depth = sizes[2];
		max_second_derivative_magnitude = -1;

		for (int i=0; i<depth; i++)
		{
			for (int j=0; j<height; j++)
			{
				for (int k=0; k<width; k++)
				{
					auto index = ((i) * height + j) * width + k;
					if (k==0 || j==0 || i==0 || k==boundary[0] || j==boundary[1] || i==boundary[2])
					{
						second_derivative_magnitude[index] = 0;
					}else
					{
						second_derivative[index].z = gradient[((i + 1) * height + j) * width + k].x - gradient[((i - 1) * height + j) * width + k].x;
						second_derivative[index].y = gradient[((i) * height + j + 1) * width + k].y - gradient[((i) * height + j - 1) * width + k].y;
						second_derivative[index].x = gradient[((i) * height + j) * width + k + 1].z - gradient[((i) * height + j) * width + k - 1].z;
						second_derivative[index] /= 2;
						second_derivative_magnitude[index] = length(second_derivative[index]);
						max_second_derivative_magnitude = std::max(second_derivative_magnitude[index], max_second_derivative_magnitude);
					}
				}
			}
		}
	}

	/// generate average and variation for scalar values
	void generate_average_variation(const int *sizes, const unsigned int count, const unsigned int components, const vector<float> &scalar_value, vector<float> &average, vector<float> &variation) 
	{
		int x, y, z, i, j, k;
		float sum;

		unsigned int index;
		int boundary[3] = {sizes[0]-1, sizes[1]-1, sizes[2]-1};
		int width = sizes[0], height = sizes[1], depth = sizes[2];

		for (i=0; i<depth; i++)
		{
			for (j=0; j<height; j++)
			{
				for (k=0; k<width; k++)
				{
					index = get_index(i, j, k, sizes);
					if (k==0 || j==0 || i==0 || k==boundary[0] || j==boundary[1] || i==boundary[2])
					{
						average[index] =  0;
					}else
					{
						sum = 0;
						for(x =i - 1; x <= i + 1; ++x)
							for(y = j - 1;y <= j + 1; ++y)
								for(z = k - 1; z <= k + 1; ++z)
								{
									sum += scalar_value[index];
								}
								sum /= 27.0;
								average[index] = sum;
					}
				}
			}
		}

		for (i=0; i<depth; i++)
		{
			for (j=0; j<height; j++)
			{
				for (k=0; k<width; k++)
				{
					index = get_index(i, j, k, sizes);
					if (k==0 || j==0 || i==0 || k==boundary[0] || j==boundary[1] || i==boundary[2])
					{
						variation[index] =  0;
					}else
					{
						sum = 0;
						for(x =i - 1; x <= i + 1; ++x)
							for(y = j - 1;y <= j + 1; ++y)
								for(z = k - 1; z <= k + 1; ++z)
								{
									sum += pow(double(scalar_value[index] - average[index]), 2.0);
								}
								sum /= 27.0;
								sum = sqrt(sum);
								variation[index] = sum;
					}
				}
			}
		}
	}

	/// gradient estimation by Sobel 3D operator
	void estimate_gradient_with_sobel_operator(unsigned short *gradient_data, const int *sizes, const unsigned int count, const vector<float> &scalar_value, vector<nv::vec3f> &gradient)
	{
		unsigned int index;
		int boundary[3] = {sizes[0]-1, sizes[1]-1, sizes[2]-1};
		int width = sizes[0], height = sizes[1], depth = sizes[2];

		for (int i=0; i<depth; i++)
		{
			for (int j=0; j<height; j++)
			{
				for (int k=0; k<width; k++)
				{
					index = get_index(i, j, k, sizes);
					if (k==0 || j==0 || i==0 || k==boundary[0] || j==boundary[1] || i==boundary[2])
					{
						gradient[index].x = gradient[index].y = gradient[index].z =  0;
					}else
					{
						gradient[index] = sobel(i, j, k, scalar_value, sizes);
					}
				}
			}
		}

		for (unsigned int i=0; i<count; i++)
		{
			auto g = nv::normalize(gradient[i]);
			auto j = i * 3;
			gradient_data[j]   = (unsigned short)(g.x * 65535);
			gradient_data[j+1] = (unsigned short)(g.y * 65535);
			gradient_data[j+2] = (unsigned short)(g.z * 65535);
		}
	}

	/// save scalar histogram to text file
	template <class T, int TYPE_SIZE>
	void save_histogram_to_text_file(const unsigned int count, unsigned int *histogram)
	{
		char *centroids_filename = "d:/histogram.txt";
		ofstream out(centroids_filename);
		if (out.is_open())
		{
			out<<TYPE_SIZE<<" "<<count<<endl;
			for (int i=0; i<TYPE_SIZE; i++)
			{
				out<<histogram[i]<<endl;
			}
		}
		out.close();
		cout<<"The histogram is saved to "<<centroids_filename<<endl;
	}

	/// save gradient to text file
	void save_gradient_to_text_file(const unsigned int type_size, const unsigned int count, vector<nv::vec3f> &gradient, const int size_x, const int size_y, const int size_z)
	{
		ofstream out("d:/gradients.txt");
		if (out.is_open())
		{
			out<<type_size<<" "<<count<<" "<<size_x<<" "<<size_y<<" "<<size_z<<endl;
			for (int i=0; i<count; i++)
			{
				out<<gradient[i].x<<" "<<gradient[i].y<<" "<<gradient[i].z<<endl;
			}
		}
		out.close();
	}

	/// save intensity to text file
	void save_intensity_to_text_file(const unsigned int type_size, const unsigned int count, const vector<float> &scalar_value, const int size_x, const int size_y, const int size_z)
	{
		ofstream out("d:/intensities.txt");
		if (out.is_open())
		{
			out<<type_size<<" "<<count<<" "<<size_x<<" "<<size_y<<" "<<size_z<<endl;
			for (int i=0; i<count; i++)
			{
				out<<scalar_value[i]<<endl;
			}
		}
		out.close();
	}

}

#endif // volume_utility_h
