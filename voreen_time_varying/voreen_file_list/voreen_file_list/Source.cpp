#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
using namespace std;

void main()
{
	string volume_str = "                                <url value=\"raw://../../../../_time_varying_data/5Jets/Velocity/ns_%.4d_v.dat?dim_x=128&amp;dim_y=128&amp;dim_z=128&amp;format=FLOAT&amp;headerskip=0&amp;objectModel=RGB&amp;spacing_x=1&amp;spacing_y=1&amp;spacing_z=1&amp;timeframe=0\" />\n";
	string select_str = "                                <entry url=\"raw://../../../../_time_varying_data/5Jets/Velocity/ns_%.4d_v.dat?dim_x=128&amp;dim_y=128&amp;dim_z=128&amp;format=FLOAT&amp;headerskip=0&amp;objectModel=RGB&amp;spacing_x=1&amp;spacing_y=1&amp;spacing_z=1&amp;timeframe=0\" value=\"true\" />\n";

	const int begin = 1, end = 100;

	FILE * f1;
	// Open for write 
	if(fopen_s( &f1, "VolumeURLs.txt", "w+" ) != 0)
		printf( "The file 'VolumeURLs.txt' was not opened\n" );
	else
		printf( "The file 'VolumeURLs.txt' was opened\n" );

	for (int i=begin; i<=end; i++)
	{
		fprintf(f1, volume_str.c_str(), i);
	}
	fclose(f1);

	FILE * f2;
	// Open for write 
	if(fopen_s( &f2, "Selection.txt", "w+" ) != 0)
		printf( "The file 'Selection.txt' was not opened\n" );
	else
		printf( "The file 'Selection.txt' was opened\n" );

	for (int i=begin; i<=end; i++)
	{
		fprintf(f2, select_str.c_str(), i);
	}
	fclose(f2);
}
