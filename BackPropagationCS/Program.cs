using NeuronNetwork.Algorithm;
using NeuronNetwork.Data;
using NeuronNetwork.Network;

using System.Drawing;

Console.WriteLine("Reading Train set");
DataSet set = MNISTReader.ReadMNISTData(@"D:/train-images.idx3-ubyte", @"D:/train-labels.idx1-ubyte", Algorithms.NormalizationMode.MinusOneToOne);

Console.WriteLine("Reading Verify set");
DataSet verify = MNISTReader.ReadMNISTData(@"D:/t10k-images.idx3-ubyte", @"D:/t10k-labels.idx1-ubyte", Algorithms.NormalizationMode.MinusOneToOne);

BackPropagationNetwork network = new BackPropagationNetwork(28 * 28, 10, 1, 32, Algorithms.ShiftedSigmoid, Algorithms.ShiftedSigmoid_D, 0.01);

network.RandomizeAllWeights();

for(int count = 0; count < 5; count++)
{
	for (int i = 0; i < set.Count(); i++)
	{
		network.TrainData(set.dataSet[i], errorThreshold:0.01);

		if (i % 100 == 99)
		Console.WriteLine("{0},{1}, err:{2:F5}", i, set.dataSet[i].label, network.loss);
	}
}

Bitmap bitmap = new Bitmap("D:/3.png");

double[] db = new double[28 * 28];

for(int y = 0; y<28; y++)
	for(int x = 0; x<28; x++)
    {
		db[y*28+x] = (double)bitmap.GetPixel(x, y).R / 255.0;
    }

network.PushData(db);
network.ForwardTransmit();
int label_out = network.FindLargestOutput();

Console.WriteLine("aaa:{0}", label_out);

Console.WriteLine(network.VerifyAccuracy(verify));