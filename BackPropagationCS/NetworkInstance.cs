using NeuronNetwork.Algorithm;
using NeuronNetwork.Data;
using NeuronNetwork.Structure;

namespace NeuronNetwork.Network
{
    public class BackPropagationNetwork
    {
        public delegate double ForwardActive(double x, double response);
        public delegate double BackwardActive(double x);

        public int inNeuronCount, outNeuronCount;

        public NeuronLayer[] hiddenLayerList;
        public NeuronLayer inLayer;
        public NeuronLayer outLayer;

        public double learningRate;

        private double[] inData;
        private double[] targetData;
        public double loss;

        ForwardActive forwardActiveFunc;
        BackwardActive backwardActiveFunc;

        public BackPropagationNetwork(int inNeuronCount, int outNeuronCount, int hiddenLayerCount, int hiddenNeuronCount, ForwardActive forwardActive, BackwardActive backwardActive, double learningRate = 0.01)
        {
            inLayer = new NeuronLayer(inNeuronCount, 0);
            outLayer = new NeuronLayer(outNeuronCount, hiddenNeuronCount);
            hiddenLayerList = new NeuronLayer[hiddenLayerCount];

            this.learningRate = learningRate;
            this.inNeuronCount = inNeuronCount;
            this.outNeuronCount = outNeuronCount;
            loss = 0.0;

            forwardActiveFunc = forwardActive;
            backwardActiveFunc = backwardActive;

            targetData = new double[outNeuronCount];
            inData = new double[inNeuronCount];

            for (int i = 0; i < hiddenLayerCount; i++)
            {
                hiddenLayerList[i] = new NeuronLayer(hiddenNeuronCount, i == 0 ? inNeuronCount : hiddenNeuronCount);
                hiddenLayerList[i].response /= hiddenLayerList[i].prevLayerNeuronCount;
            }
        }

        public void GetLoss()
        {
            loss = 0.0;

            for (int i = 0; i < outLayer.neurons.Length; i++)
            {
                loss += (outLayer.neurons[i].value - targetData[i]) * (outLayer.neurons[i].value - targetData[i]);
            }

            loss /= outLayer.neurons.Length;
        }

        public void RandomizeAllWeights()
        {
            foreach (NeuronLayer layer in hiddenLayerList)
            {
                Algorithms.RandomDist(layer);
            }

            Algorithms.RandomDist(outLayer);
        }

        public void SetAllWeights(double weight)
        {
            foreach(NeuronLayer layer in hiddenLayerList)
            {
                for(int i = 0; i<layer.Count(); i++)
                {
                    layer.neurons[i].InitWeights(weight);
                }
            }

            for (int i = 0; i < outLayer.Count(); i++)
            {
                outLayer.neurons[i].InitWeights(weight);
            }
        }

        public void PushData(double[] data)
        {
            if (data.Length != inNeuronCount)
                throw new InvalidDataException("Data size doesn't match!");

            inData = data;

            for (int i = 0; i < inNeuronCount; i++)
            {
                inLayer.neurons[i].value = data[i];
            }
        }

        public void PushTarget(double[] target)
        {
            if (target.Length != outNeuronCount)
                throw new InvalidDataException("Data size doesn't match!");

            targetData = target;
        }

        public void TrainData(NetworkData data, int maxIterCount = 3, double errorThreshold = 0.05)
        {
            PushData(data.data);

            targetData = new double[outNeuronCount];

            for (int i = 0; i < targetData.Length; i++)
            {
                targetData[i] = i == data.label ? 1.0 : 0.0;
            }

            for (int iterCount = 0; iterCount < maxIterCount; iterCount++)
            {
                ForwardTransmit();

                GetLoss();

                if (loss < errorThreshold)
                {
                    break;
                }

                BackwardTransmit();
                WeightUpdate();
            }
        }

        public void ForwardTransmit()
        {
            for (int i = 0; i < hiddenLayerList.Length; i++)
            {
                NeuronLayer prevLayer = i == 0 ? inLayer : hiddenLayerList[i - 1];
                NeuronLayer layer = hiddenLayerList[i];

                for (int j = 0; j < layer.Count(); j++)
                {
                    layer.neurons[j].value = 0;

                    for (int m = 0; m < layer.prevLayerNeuronCount; m++)
                    {
                        layer.neurons[j].value += prevLayer.neurons[m].value * layer.neurons[j].weights[m];
                    }

                    layer.neurons[j].value += layer.bias;
                    layer.neurons[j].value = forwardActiveFunc(layer.neurons[j].value, layer.response);
                }
            }

            NeuronLayer lastHiddenLayer = hiddenLayerList.Last();

            for (int j = 0; j < outLayer.Count(); j++)
            {
                outLayer.neurons[j].value = 0;

                for (int m = 0; m < outLayer.prevLayerNeuronCount; m++)
                {
                    outLayer.neurons[j].value += lastHiddenLayer.neurons[m].value * outLayer.neurons[j].weights[m];
                }

                outLayer.neurons[j].value += outLayer.bias;
                outLayer.neurons[j].value /= outLayer.prevLayerNeuronCount;
            }

            outLayer.SoftMax();
        }

        public void BackwardTransmit()
        {
            outLayer.SoftMax_GetError(targetData);

            for (int i = hiddenLayerList.Length - 1; i >= 0; i--)
            {
                NeuronLayer lastLayer = i == hiddenLayerList.Length - 1 ? outLayer : hiddenLayerList[i + 1];
                NeuronLayer layer = hiddenLayerList[i];

                for (int j = 0; j < layer.neurons.Length; j++)
                {
                    layer.neurons[j].error = 0;

                    for (int m = 0; m < lastLayer.Count(); m++)
                    {
                        layer.neurons[j].error += lastLayer.neurons[m].error * lastLayer.neurons[m].weights[j];
                    }
                }
            }
        }

        private void UpdateLayer(ref NeuronLayer target, NeuronLayer last)
        {
            for(int i = 0; i < target.Count(); i++) // for each neuron in layer
            {
                double temp = learningRate * backwardActiveFunc(target.neurons[i].value) * target.neurons[i].error; // extract common coeffs

                target.bias += learningRate * backwardActiveFunc(target.bias) * target.neurons[i].error;

                for (int j = 0; j < target.neurons[i].WeightCount(); j++) // for each weight in neuron
                {
                    target.neurons[i].weights[j] += temp * last.neurons[j].value ;
                }
            }
        }

        public void WeightUpdate()
        {
            UpdateLayer(ref outLayer, hiddenLayerList.Last());

            for(int i = 0; i < hiddenLayerList.Count(); i++)
            {
                UpdateLayer(ref hiddenLayerList[i], i == 0 ? inLayer : hiddenLayerList[i - 1]);
            }
        }

        public int FindLargestOutput()
        {
            double biggest = outLayer.neurons[0].value;
            int biggestN = 0;

            for(int i = 1; i<outLayer.Count();i++)
            {
                if(outLayer.GetValue(i) > biggest)
                {
                    biggest = outLayer.GetValue(i);
                    biggestN = i;
                }
            }

            return biggestN;
        }

        public double VerifyAccuracy(DataSet set)
        {
            int correctCount = 0;
            int[] correctCountByLabel = new int[outNeuronCount];

            foreach(NetworkData data in set.dataSet)
            {
                PushData(data.data);
                ForwardTransmit();

                int getLabel = FindLargestOutput();

                if (getLabel == data.label)
                {
                    correctCount++;
                    correctCountByLabel[getLabel]++;
                }
            }

            foreach(int i in correctCountByLabel)
            {
                Console.Write("{0},", i);
            }

            return (double)correctCount / (double)set.Count();
        }
    }
}
