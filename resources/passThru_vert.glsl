void main()
{
//	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
	gl_Position.x += 5.;
}
