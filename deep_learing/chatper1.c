#include <stdio.h>

#define MAX(x,b) (a) > (b) ? (a) : (b)

double w,b;

int getActivation(double x)
{
  return x;
}

double feedforward(double input)
{
  double sigma;
  double activation;
  double output;

  sigma= w*input+b;
  activation=getActivation(sigma);
  output = activation;

  return output;
}


int main(int argc, const char *argv[])
{
  double input;
  w=2.0;
  b=1.0;

  input=10.0;
  fprintf(stderr,"%f\n",feedforward(input));

  input=12.0;
  fprintf(stderr,"%f\n",feedforward(input));
 
  input=13.0;
  fprintf(stderr,"%f\n",feedforward(input));





  return 0;
}
