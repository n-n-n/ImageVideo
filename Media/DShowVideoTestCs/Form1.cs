using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using CommonManage;

namespace DShowVideoTestCs
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }
        private void setSize(int w, int h)
        {
            this.Width = w;
            this.Height = h;
            this.pictureBox1.Width = w;
            this.pictureBox1.Height = h;
        }

        private void btnVideo_Click(object sender, EventArgs e)
        {
            DeleteCapture();
            var dialog = new OpenFileDialog()
            {
                Title = "Open Movie File",
                Filter = ".avi, .mpg, .wmv|*.avi;*.mpg;*.wmv|all|*.*"
            };
            if (dialog.ShowDialog(this) == DialogResult.OK)
            {

                DeleteCapture();
                video = new VideoCapture(dialog.FileName, this.pictureBox1, VideoCapture.VMR.VMR9);
                setSize(video.Width, video.Height);
                video.Play();
            }
        }

        private void DeleteCapture()
        {
            if (null != video)
            {
                video.Dispose();
                video = null;
            }
            if (null != camera)
            {
                camera.Dispose();
                camera = null;
            }
        }
        private VideoCapture video = null;
        private CameraCaptureDevice cameraDevice = null;
        private CameraCapture camera = null;

        private void Form1_Shown(object sender, EventArgs e)
        {
            cameraDevice = new CameraCaptureDevice();
            comboBoxCamera.Items.AddRange(cameraDevice.GetDevice().ToArray());
            if (comboBoxCamera.Items.Count > 0) comboBoxCamera.SelectedIndex = 0;
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            DeleteCapture();
        }

        private void pictureBox1_SizeChanged(object sender, EventArgs e)
        {
            if (null != video) video.SetCtrl(this.pictureBox1);
        }

        private void btnCamera_Click(object sender, EventArgs e)
        {
            DeleteCapture();
            var hEvent = IntPtr.Zero;
            int index = comboBoxCamera.SelectedIndex;
            camera = cameraDevice.Create(index, this.pictureBox1, hEvent);
            MethodInvoker invoked_method = (() =>
                                {
                                    if (null != pictureBox1.Image) pictureBox1.Image.Dispose();
                                    if (null != camera)
                                    {
                                        var img = camera.SnapShot().ToBMP();
                                        this.pictureBox1.Image = img;
                                    }
                                });
            unsafe
            {
                camera.SampleEvent = (time, buf, size) =>
                {
                    BeginInvoke((MethodInvoker)invoked_method);
                };
            }
            camera.ResetSampleEvent();
            setSize(camera.Width, camera.Height);
            camera.Start();
        }

    }
}
