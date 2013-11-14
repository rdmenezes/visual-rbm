using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace VisualRBM
{
	public enum PixelFormat
	{
		Lightness,
		RGB
	};

	public partial class ImageControl : UserControl
	{
		public PixelFormat Format;

		// dimensions of the bitmap to display
		private int image_width;
		private int image_height;
		private int scale;
		private int count;
		// our raw pixel data we use to populate a bitmap
		private byte[][] raw_bmp = null;

		private const int border_width = 1;

		public ImageControl(int in_count, int in_image_width, int in_image_height, int in_scale, PixelFormat in_pixel_format)
		{
			image_width = in_image_width;
			image_height = in_image_height;
			scale = in_scale;
			count = in_count;

			raw_bmp = new byte[in_count][];
			for (int k = 0; k < in_count; k++)
				raw_bmp[k] = new byte[image_width * image_height * 4];

			this.Format = in_pixel_format;
			this.Width = image_width * scale + 2 * border_width;
			this.Height = image_height * scale * count + (count + 1) * border_width;

			InitializeComponent();
			SetStyle(ControlStyles.OptimizedDoubleBuffer | ControlStyles.UserPaint | ControlStyles.AllPaintingInWmPaint, true);
		}



		public void UpdateSize(int in_image_width, int in_image_height, int in_scale)
		{
			lock (raw_bmp)
			{
				int count = raw_bmp.Length;
				if (image_width != in_image_width || image_height != in_image_height)
				{
					image_width = in_image_width;
					image_height = in_image_height;

					

					for(int k = 0; k < count; k++)
					{
						byte[] temp = new byte[image_width * image_height * 4];
						Array.Copy(raw_bmp[k], temp, Math.Min(raw_bmp.Length, temp.Length));
						raw_bmp[k] = temp;
					}
				}
				scale = in_scale;

				this.SuspendLayout();
				this.Width = image_width * scale + 2 * border_width;
				this.Height = image_height * scale * count + (count + 1) * border_width;
				this.ResumeLayout();
			}
		}

		public unsafe void SetImage(int which, uint length, float* image_data)
		{
			if (image_data == null)
				return;

			lock (raw_bmp[which])
			{
				byte alpha = 255;

				if (Format == PixelFormat.Lightness)
				{
					for (int k = 0; k < length; k++)
					{
						if ((4 * k + 3) >= raw_bmp[which].Length)
							break;

						byte red, green, blue;

						byte brightness = (byte)(image_data[k] * 255.0f);
						red = green = blue = brightness;



						raw_bmp[which][4 * k + 3] = alpha;
						raw_bmp[which][4 * k + 2] = red;
						raw_bmp[which][4 * k + 1] = green;
						raw_bmp[which][4 * k + 0] = blue;
					}
				}
				else if (Format == PixelFormat.RGB)
				{
					int index = 0;
					for (int k = 0; k + 3 <= length; )
					{
						if (index + 4 > raw_bmp[which].Length)
							break;

						byte red, green, blue;
						red = (byte)(image_data[k++] * 255.0f);
						green = (byte)(image_data[k++] * 255.0f);
						blue = (byte)(image_data[k++] * 255.0f);

						raw_bmp[which][index++] = blue;
						raw_bmp[which][index++] = green;
						raw_bmp[which][index++] = red;
						raw_bmp[which][index++] = alpha;
					}
				}
			}
		}

		public void SetImage(int which, float[] image_data)
		{
			if (image_data == null)
				return;

			lock (raw_bmp[which])
			{
				byte alpha = 255;

				if (Format == PixelFormat.Lightness)
				{
					for (int k = 0; k < image_data.Length; k++)
					{
						if ((4 * k + 3) >= raw_bmp[which].Length)
							break;

						byte red, green, blue;

						byte brightness = (byte)(image_data[k] * 255.0f);
						red = green = blue = brightness;



						raw_bmp[which][4 * k + 3] = alpha;
						raw_bmp[which][4 * k + 2] = red;
						raw_bmp[which][4 * k + 1] = green;
						raw_bmp[which][4 * k + 0] = blue;
					}
				}
				else if (Format == PixelFormat.RGB)
				{
					int index = 0;
					for (int k = 0; k + 3 <= image_data.Length;)
					{
						if (index + 4 > raw_bmp[which].Length)
							break;

						byte red, green, blue;
						red = (byte)(image_data[k++] * 255.0f);
						green = (byte)(image_data[k++] * 255.0f);
						blue = (byte)(image_data[k++] * 255.0f);

						raw_bmp[which][index++] = blue;
						raw_bmp[which][index++] = green;
						raw_bmp[which][index++] = red;
						raw_bmp[which][index++] = alpha;
					}
				}
			}
		}

		protected override void OnPaint(PaintEventArgs e)
		{
			if (raw_bmp == null) return;

			unsafe
			{
				e.Graphics.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
				e.Graphics.PixelOffsetMode = System.Drawing.Drawing2D.PixelOffsetMode.Half;
				e.Graphics.FillRectangle(new SolidBrush(Color.Black), new Rectangle(0, 0, this.Width, this.Height));
					
				int width = image_width * scale;
				int height = image_height * scale;

					


				for (int k = 0; k < raw_bmp.Length; k++)
				{
					lock (raw_bmp)
					{
						fixed (byte* p = raw_bmp[k])
						{
							Bitmap bmp = new Bitmap(image_width, image_height, image_width * 4, System.Drawing.Imaging.PixelFormat.Format32bppArgb, (IntPtr)p);
							e.Graphics.DrawImage(bmp, border_width, border_width + k * (height + border_width), width, height);
						}
					}
				}
			}
		}
	}
}
