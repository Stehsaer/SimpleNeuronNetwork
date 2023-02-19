using NeuronNetwork.Algorithm;
using static System.ArrayExtension;

namespace NeuronNetwork.Data
{
    public struct NetworkData
    {
        public double[] data;
        public int label;

        public NetworkData(double[] data, int label)
        {
            this.data = data;
            this.label = label;
        }

        // Display the data as image in the console, some grayscale supported!
        public void ShowImageConsole(int width, Algorithm.Algorithms.NormalizationMode mode)
        {
            Console.WriteLine(String.Format("Displaying data with label {0}", label));

            string[] chars = { "  ", "░░", "▒▒", "▓▓", "██" };

            Console.OutputEncoding = System.Text.Encoding.Unicode;

            if (mode == Algorithm.Algorithms.NormalizationMode.ZeroToOne)
            {
                for (int i = 0; i < data.Length; i++)
                {
                    string c = chars[(int)(data[i] * 255.0) / 64];
                    Console.Write(c);

                    if (i % width == width - 1)
                    {
                        Console.Write("\n");
                    }
                }
            }
        }
    }

    public class DataSet
    {
        public string name;
        public List<NetworkData> dataSet;

        public DataSet(string name = "")
        {
            this.name = name;
            dataSet = new List<NetworkData>();
        }

        // Get data count in the set
        public int Count()
        {
            return dataSet.Count();
        }

        // Add a data into the set
        public void AddData(NetworkData data)
        {
            dataSet.Add(data);
        }
    }

    public class MNISTReader
    {
        const int DataCountOffset = 4;
        const int DataWidthOffset = 8;
        const int DataHeightOffset = 12;
        const int DataOffset = 16;

        const int LabelCountOffset = 4;
        const int LabelOffset = 8;

        const int DataMagicNumber = 2051;
        const int LabelMagicNumber = 2049;

        public static DataSet ReadMNISTData(string dataPath, string labelPath, Algorithms.NormalizationMode mode = Algorithms.NormalizationMode.ZeroToOne)
        {
            // Detect if file exists
            if (!File.Exists(dataPath) || !File.Exists(labelPath))
            {
                throw new IOException("File doesn't exist!");
            }

            // fetch data from file
            byte[] dataFile = File.ReadAllBytes(dataPath);
            byte[] labelFile = File.ReadAllBytes(labelPath);

            // get magic number
            int DataMagicNumber_Read = Algorithm.Algorithms.GetIntFromBytes(dataFile, 0, true);
            int LabelMagicNumber_Read = Algorithm.Algorithms.GetIntFromBytes(labelFile, 0, true);

            // throws an exception if magic numbers dont match
            if (DataMagicNumber_Read != DataMagicNumber || LabelMagicNumber_Read != LabelMagicNumber)
            {
                throw new InvalidDataException(String.Format("Invalid data! ({0},{1})", DataMagicNumber_Read, LabelMagicNumber_Read));
            }

            // get counts in data and labels
            int dataCount = Algorithms.GetIntFromBytes(dataFile, DataCountOffset, true);
            int labelCount = Algorithms.GetIntFromBytes(labelFile, LabelCountOffset, true);

            // checks if two counts match
            if (dataCount != labelCount)
            {
                throw new InvalidDataException("Data size mismatch!");
            }

            // head on
            DataSet outDataSet = new DataSet();
            int dataWidth = Algorithms.GetIntFromBytes(dataFile, DataWidthOffset, true);
            int dataHeight = Algorithms.GetIntFromBytes(dataFile, DataHeightOffset, true);

            // save data into dataset one by one
            for (int i = 0; i < dataCount; i++)
            {
                outDataSet.AddData(new NetworkData(
                    Algorithms.Normalization(
                        Crop(dataFile, DataOffset + i * dataWidth * dataHeight, dataWidth * dataHeight), mode),
                    labelFile[LabelOffset + i]
                    )
                );
            }

            return outDataSet;
        }
    }
}
