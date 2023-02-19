using NeuronNetwork.Structure;
using static System.Math;

namespace NeuronNetwork.Algorithm
{
    public class Algorithms
    {
        public enum NormalizationMode
        {
            ZeroToOne = 0,
            MinusOneToOne = 1
        };

        public static double Sigmoid(double x, double response)
        {
            return 1.0 / (1.0 + Exp(-x * response));
        }

        public static double Sigmoid_D(double x)
        {
            return x * (1.0 - x);
        }

        public static double ShiftedSigmoid(double x, double response)
        {
            return Sigmoid(x, response) * 2.0 - 1.0;
        }

        public static double ShiftedSigmoid_D(double x)
        {
            return Sigmoid_D((x + 1.0) / 2.0);
        }

        public static double ReLU(double x, double response)
        {
            double _x = x * response;
            return _x > 0.0 ? _x : 0.0;
        }

        public static double ReLU_D(double x)
        {
            return x > 0.0 ? 1.0 : 0.0;
        }

        public static double[] Normalization(byte[] data, NormalizationMode mode)
        {
            double[] o = new double[data.Length];

            if (mode == NormalizationMode.ZeroToOne)
            {
                for (int i = 0; i < o.Length; i++)
                {
                    o[i] = data[i] / 255.0;
                }
            }
            else
            {
                for (int i = 0; i < o.Length; i++)
                {
                    o[i] = data[i] / 127.5 - 1.0;
                }
            }

            return o;
        }

        public static int GetIntFromBytes(byte[] data, int offset, bool MSB = false)
        {
            byte[] _data = ArrayExtension.Crop(data, offset, sizeof(int));

            if (MSB) Array.Reverse(_data);

            return BitConverter.ToInt32(_data, 0);
        }

        public static void RandomDist(NeuronLayer layer, double min = 0.0, double max = 1.0)
        {
            Random rnd = new Random(Environment.TickCount);
            for (int i = 0; i < layer.neurons.Length; i++)
            {
                for (int j = 0; j < layer.neurons[i].weights.Length; j++)
                {
                    layer.neurons[i].weights[j] = CastToRange(rnd.NextDouble(), min, max);
                }
            }
        }

        public static double CastToRange(double d, double min, double max)
        {
            return min + d * (max - min);
        }
    }
}

namespace System
{
    public class ArrayExtension
    {
        public static T[] Crop<T>(T[] array, int start, int length)
        {
            return array.Skip(start).Take(length).ToArray();
        }
    }
}

namespace NeuronNetwork.Structure
{
    public partial class NeuronLayer
    {
        public void SoftMax()
        {
            double sum = 0.0;
            double biggestValue = neurons[0].value;

            for (int i = 0; i < neurons.Length; i++)
            {
                if (neurons[i].value > biggestValue)
                    biggestValue = neurons[i].value;
            }

            for (int i = 0; i < neurons.Length; i++)
            {
                sum += Exp(neurons[i].value - biggestValue);
            }

            for (int i = 0; i < neurons.Length; i++)
            {
                neurons[i].value = Exp(neurons[i].value - biggestValue) / sum;
            }
        }

        public void SoftMax_GetError(double[] target)
        {
            if(target.Length != neurons.Length)
            {
                throw new Exception("Target datasize mismatch!");
            }

            for(int i = 0; i < neurons.Length; i++)
            {
                neurons[i].error = target[i] - neurons[i].value;
            }
        }
    }
}
