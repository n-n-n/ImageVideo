
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Threading.Tasks;
using System.IO;

class BmpCount
{
    unsafe static void CountCore(ref Tuple<double, double, double> ret, byte* p, int width, int height, int stride)
    {
        double a = 0.0, b = 0.0, c = 0.0;
        for (int y = 0; y < height; y++)
        {
            int line = y * stride;
            for (int x = 0; x < width; x++)
            {
                int index = line + 4 * x;

                a += (double)p[index+ 2];
                b += (double)p[index + 1];
                c += (double)p[index];
            }
        }
        ret = new Tuple<double, double, double>(a, b, c);
    }

    unsafe static Tuple<double, double, double> Count(string file)
    {
        var bmp = new Bitmap(file);
        var rets = new Tuple<double, double, double>[4];
        double x = 0.0, y = 0.0, z = 0.0;

        BitmapData data = bmp.LockBits(new Rectangle(new Point(0, 0), bmp.Size), ImageLockMode.ReadOnly, PixelFormat.Format32bppRgb);
        byte* p = (byte*)data.Scan0.ToPointer();

        int unit_height = data.Height / 4;
        int unit = unit_height * data.Stride;
        Parallel.Invoke(
            () => CountCore(ref rets[0], p, data.Width, unit_height, data.Stride),
            () => CountCore(ref rets[1], p + unit, data.Width, unit_height, data.Stride),
            () => CountCore(ref rets[2], p + 2 * unit, data.Width, unit_height, data.Stride),
            () => CountCore(ref rets[3], p + 3 * unit, data.Width, data.Height - 3 * unit_height, data.Stride)
        );

        bmp.UnlockBits(data); 
        foreach (var item in rets)
        {
            x += item.Item1;
            y += item.Item2;
            z += item.Item3;
        }
/*

        for (int i = 0; i < bmp.Width; i++)
        {
            for (int j = 0; j < bmp.Height; j++)
            {
                Color c = bmp.GetPixel(i, j);
                x += (double)c.R;
                y += (double)c.G;
                z += (double)c.B;
            }
        }
  */

        Console.WriteLine(((int)x).ToString() + "," + ((int)y).ToString() + "," + ((int)z).ToString());
        double area = bmp.Width * bmp.Height;
        return new Tuple<double, double, double>(x/area, y/area, z/area);
    }
    static double len(Tuple<double, double, double> x, Tuple<double, double, double> y)
    {
        double ret = 0.0;
        ret += Math.Pow(x.Item1 - y.Item1, 2);
        ret += Math.Pow(x.Item2 - y.Item2, 2);
        ret += Math.Pow(x.Item3 - y.Item3, 2);
        return Math.Sqrt(ret);
    }
    static void Main(string[] args)
    {
        var reader = new StreamReader(@"E:\Resources\Tmp\Work\Tips\tmp.txt");
        string line;
        var info = new Dictionary<string, Tuple<double, double, double>>();
 
        while ((line = reader.ReadLine()) != null)
        {
            var items = line.Split(',');
            var name = items[0].Trim();
            var path = @"E:\Resources\Tmp\Work\Tips\" + items[2].Trim();
            if (System.IO.File.Exists(path))
            {
                var ret = Count(path);
                info.Add(name, ret);
                Console.WriteLine(path + ": " + ((int)ret.Item1).ToString() + "," + ((int)ret.Item2).ToString() + "," + ((int)ret.Item3).ToString());
            }
        }
        var check = info["Argentina"];

        var q = info.OrderBy(x => len(x.Value, check)).Take(5);
        foreach (var item in q)
        {
            Console.WriteLine(item.Key);
        }
 
        Console.ReadKey();
    }
}