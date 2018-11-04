// Modified from my original code at
// 
//		https://github.com/CobaltXII/boiler/blob/master/mandelbrot/burning.cpp

#version 330 core

out vec4 glx_FragColor;

uniform vec2 glx_Resolution;

uniform vec2 glx_Real;
uniform vec2 glx_Imag;

uniform vec2 glx_Factor;

float max_iter = 512.0f;

void main()
{
	float c_im = glx_Imag.x + (glx_Resolution.y - gl_FragCoord.y) * glx_Factor.y;

	float c_re = glx_Real.x + gl_FragCoord.x * glx_Factor.x;

	float z_re = c_re;
	float z_im = c_im;

	bool z_in = true;

	float n;

	for (n = 0.0f; n < max_iter; n += 1.0f)
	{
		float z_re2 = z_re * z_re;
		float z_im2 = z_im * z_im;

		if (z_re2 + z_im2 > 4.0)
		{
			z_in = false;

			break;
		}

		z_im = abs(2.0 * z_re * z_im + c_im);

		z_re = abs(z_re2 - z_im2 + c_re);
	}

	if (z_in == true)
	{
		glx_FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);

		return;
	}
	else
	{
		{
			float z_re2 = z_re * z_re;
			float z_im2 = z_im * z_im;

			z_im = abs(2.0 * z_re * z_im + c_im);

			z_re = abs(z_re2 - z_im2 + c_re);

			n += 1.0f;
		}

		{
			float z_re2 = z_re * z_re;
			float z_im2 = z_im * z_im;

			z_im = abs(2.0 * z_re * z_im + c_im);

			z_re = abs(z_re2 - z_im2 + c_re);

			n += 1.0f;
		}

		float modulus = sqrt(z_re * z_re + z_im * z_im);

		n = n - (log(log(modulus))) / log(2.0f);

		n = pow(n / max_iter, 0.5) * max_iter;

		float t = n / max_iter;

		glx_FragColor = vec4
		(
			9.0f * (1.0f - t) * t * t * t,

			15.0f * (1.0f - t) * (1.0f - t) * t * t,
			
			8.5f * (1.0f - t) * (1.0f - t) * (1.0f - t) * t,

			1.0f
		);

		return;
	}
}