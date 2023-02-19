using static System.Math;

namespace NeuronNetwork.Structure
{
    public struct Neuron
    {
        public double value;
        public double error;

        public double[] weights;

        public Neuron(int weightCount)
        {
            weights = new double[weightCount];
            value = 0.0;
            error = 0.0;
        }

        public void InitWeights(double initValue)
        {
            for (int i = 0; i < weights.Length; i++)
            {
                weights[i] = initValue;
            }
        }

        public double WeightCount()
        {
            return weights.Length;
        }
    }

    public partial class NeuronLayer
    {
        public Neuron[] neurons;
        public int prevLayerNeuronCount;

        public double response, bias;

        public NeuronLayer(int neuronCount, int prevLayerNeuronCount)
        {
            this.prevLayerNeuronCount = prevLayerNeuronCount;
            this.response = 1.0;
            this.bias = 0.0;

            this.neurons = new Neuron[neuronCount];

            for (int i = 0; i < neuronCount; i++)
            {
                neurons[i] = new Neuron(prevLayerNeuronCount);
                neurons[i].InitWeights(0.0);
            }
        }

        public int Count()
        {
            return neurons.Length;
        }

        public double GetValue(int index)
        {
            return neurons[index].value;
        }

        public void Debug_ShowValues()
        {
            Console.Write("[");

            foreach(Neuron neu in neurons)
            {
                Console.Write("{0},", neu.value);
            }

            Console.WriteLine("\b]");
        }
    }
}