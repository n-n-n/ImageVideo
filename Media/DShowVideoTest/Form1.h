#pragma once
#include <windows.h>
#include "VideoCapture.h"

#define USE_CAMERA
#ifdef USE_CAMERA
#include "CameraCapture.h"
#endif

namespace DShowVideoTest {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	using namespace CommonManage;
	/// <summary>
	/// Form1 �̊T�v
	///
	/// �x��: ���̃N���X�̖��O��ύX����ꍇ�A���̃N���X���ˑ����邷�ׂĂ� .resx �t�@�C���Ɋ֘A�t����ꂽ
	///          �}�l�[�W ���\�[�X �R���p�C�� �c�[���ɑ΂��� 'Resource File Name' �v���p�e�B��
	///          �ύX����K�v������܂��B���̕ύX���s��Ȃ��ƁA
	///          �f�U�C�i�ƁA���̃t�H�[���Ɋ֘A�t����ꂽ���[�J���C�Y�ς݃��\�[�X�Ƃ��A
	///          ���������݂ɗ��p�ł��Ȃ��Ȃ�܂��B
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
		{
			InitializeComponent();
			//
			//TODO: �����ɃR���X�g���N�^ �R�[�h��ǉ����܂�
			//
			date_time = DateTime::Now;
            cameraDevice = gcnew CameraCaptureDevice();
            comboBoxCamera->Items->AddRange(cameraDevice->GetDevice()->ToArray());
            if (comboBoxCamera->Items->Count > 0) comboBoxCamera->SelectedIndex = 0;
		}

	protected:
        /// <summary>
        /// �g�p���̃��\�[�X�����ׂăN���[���A�b�v���܂��B
        /// </summary>
        ~Form1()
        {
            DeleteCapture();

            if (components)
            {
                delete components;
            }
        }
    private: System::Windows::Forms::Button^  button1;
    private: System::Windows::Forms::PictureBox^  pictureBox1;
    private: System::Windows::Forms::Button^  button2;
    private: System::Windows::Forms::Button^  button3;
    private: System::Windows::Forms::Label^  label1;

    private: System::Windows::Forms::ComboBox^  comboBoxCamera;


    protected: 

    private:
        /// <summary>
        /// �K�v�ȃf�U�C�i�ϐ��ł��B
        /// </summary>
        System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
        /// <summary>
        /// �f�U�C�i �T�|�[�g�ɕK�v�ȃ��\�b�h�ł��B���̃��\�b�h�̓��e��
        /// �R�[�h �G�f�B�^�ŕύX���Ȃ��ł��������B
        /// </summary>
        void InitializeComponent(void)
        {
            this->button1 = (gcnew System::Windows::Forms::Button());
            this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
            this->button2 = (gcnew System::Windows::Forms::Button());
            this->button3 = (gcnew System::Windows::Forms::Button());
            this->label1 = (gcnew System::Windows::Forms::Label());
            this->comboBoxCamera = (gcnew System::Windows::Forms::ComboBox());
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->BeginInit();
            this->SuspendLayout();
            // 
            // button1
            // 
            this->button1->Location = System::Drawing::Point(2, 1);
            this->button1->Name = L"button1";
            this->button1->Size = System::Drawing::Size(75, 23);
            this->button1->TabIndex = 0;
            this->button1->Text = L"Video";
            this->button1->UseVisualStyleBackColor = true;
            this->button1->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
            // 
            // pictureBox1
            // 
            this->pictureBox1->Dock = System::Windows::Forms::DockStyle::Fill;
            this->pictureBox1->Location = System::Drawing::Point(0, 0);
            this->pictureBox1->Name = L"pictureBox1";
            this->pictureBox1->Size = System::Drawing::Size(581, 393);
            this->pictureBox1->TabIndex = 1;
            this->pictureBox1->TabStop = false;
            this->pictureBox1->SizeChanged += gcnew System::EventHandler(this, &Form1::pictureBox1_SizeChanged);
            // 
            // button2
            // 
            this->button2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->button2->Location = System::Drawing::Point(506, 0);
            this->button2->Name = L"button2";
            this->button2->Size = System::Drawing::Size(75, 23);
            this->button2->TabIndex = 2;
            this->button2->Text = L"Camera";
            this->button2->UseVisualStyleBackColor = true;
            this->button2->Click += gcnew System::EventHandler(this, &Form1::button2_Click);
            // 
            // button3
            // 
            this->button3->Location = System::Drawing::Point(198, 1);
            this->button3->Name = L"button3";
            this->button3->Size = System::Drawing::Size(75, 23);
            this->button3->TabIndex = 3;
            this->button3->Text = L"Snap";
            this->button3->UseVisualStyleBackColor = true;
            this->button3->Click += gcnew System::EventHandler(this, &Form1::button3_Click);
            // 
            // label1
            // 
            this->label1->AutoSize = true;
            this->label1->BackColor = System::Drawing::Color::Transparent;
            this->label1->Location = System::Drawing::Point(309, 6);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(35, 12);
            this->label1->TabIndex = 4;
            this->label1->Text = L"label1";
            // 
            // comboBoxCamera
            // 
            this->comboBoxCamera->FormattingEnabled = true;
            this->comboBoxCamera->Location = System::Drawing::Point(379, 1);
            this->comboBoxCamera->Name = L"comboBoxCamera";
            this->comboBoxCamera->Size = System::Drawing::Size(121, 20);
            this->comboBoxCamera->TabIndex = 6;
            // 
            // Form1
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(581, 393);
            this->Controls->Add(this->comboBoxCamera);
            this->Controls->Add(this->label1);
            this->Controls->Add(this->button3);
            this->Controls->Add(this->button2);
            this->Controls->Add(this->button1);
            this->Controls->Add(this->pictureBox1);
            this->Name = L"Form1";
            this->Text = L";";
            this->Shown += gcnew System::EventHandler(this, &Form1::Form1_Shown);
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->EndInit();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion
    private: 
        System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) 
        {
            //����t�@�C���̓ǂݍ��݂悤�_�C�A���O�̍쐬
            OpenFileDialog^ openFileDialog = gcnew OpenFileDialog();
            openFileDialog->Title = gcnew String("����t�@�C�����J��");
            openFileDialog->Filter = gcnew String(".avi, .mpg, .wmv|*.avi;*.mpg;*.wmv|���ׂẴt�@�C��|*.*");

            //�����̃_�C�A���O��OK��Ԃ����ꍇ
            if(openFileDialog->ShowDialog(this) == Windows::Forms::DialogResult::OK)
            {
                Start(openFileDialog->FileName);
            }
        }
        System::Void pictureBox1_SizeChanged(System::Object^  sender, System::EventArgs^  e) 
        {
            if (video != nullptr) {
                video->SetCtrl(this->pictureBox1);
            }
        }

    protected:
        VideoCapture^ video;
#ifdef USE_CAMERA
        CameraCaptureDevice^ cameraDevice;
        CameraCapture^ camera;
#endif /* USE_CAMERA */

        DateTime date_time;
        void DeleteCapture() 
        {
            if (video != nullptr) {
                delete video;
                video = nullptr;
            }
#ifdef USE_CAMERA
            if (camera != nullptr) {
                delete camera;
                camera = nullptr;
            }
#endif /* USE_CAMERA */

        }
        void UpdateSize(int width, int height)
        {
            this->pictureBox1->Width = width;
            this->pictureBox1->Height = height;
            this->Width = width;
            this->Height = height;
        }

        void Start(String^ file)
        {
            DeleteCapture();
#ifndef NOT_DIRECT
            video = gcnew VideoCapture(file, this->pictureBox1, VideoCapture::VMR::VMR9);

#else
            video = gcnew VideoCapture(file, this->pictureBox2->Handle, VideoCapture::VMR::VMR9);
            video->SampleEvent = gcnew VideoCapture::SampleEventHandler(this, &Form1::VideoSampleUpdate);
#endif
            UpdateSize(video->Width, video->Height);
            // video->DisableClock(); // Clock�����𖳌�������(�\���͑����Ȃ�)
            video->Play();
        }

        void VideoSampleUpdate(double dTime, BYTE* pBuf, int nBuf)
        {
            if(this->InvokeRequired)
            {
                BeginInvoke(gcnew Form1::UpdateEventHandler(this, &Form1::ImageUpdateEvent));
                return;
            }
        }
#ifdef USE_CAMERA
        void CameraSampleUpdate(double dTime, BYTE* pBuf, int nBuf)
        {
            if(this->InvokeRequired)
            {
                BeginInvoke(gcnew Form1::UpdateEventHandler(this, &Form1::ImageUpdateEvent));
                return;
            }
        }
#endif /* USE_CAMERA */

        delegate void UpdateEventHandler();
        void ImageUpdateEvent()
        {
            RawImage^ img;
            if (nullptr != video) {
                img = video->SnapShot();
            } 
#ifdef USE_CAMERA
            else if (nullptr != camera) {
                img = camera->SnapShot();
            }
#endif /* USE_CAMERA */

            DateTime now = DateTime::Now;
            label1->Text = (1000.0 / (now - date_time).Milliseconds).ToString("F2") + "FPS";
            date_time = now;

            if (img != nullptr) {
                delete pictureBox1->Image;
                pictureBox1->Image = img->ToBMP();
            }

        }
private: 
    System::Void button2_Click(System::Object^  sender, System::EventArgs^  e)
    {
#ifdef USE_CAMERA
        DeleteCapture();

        CameraCaptureDevice^ device = gcnew CameraCaptureDevice();
        IntPtr hEvent = IntPtr::Zero;
        int index = comboBoxCamera->SelectedIndex;
        camera = device->Create(index, this->pictureBox1, hEvent);
        camera->SampleEvent = gcnew CameraCapture::SampleEventHandler(this, &Form1::CameraSampleUpdate);
        camera->ResetSampleEvent();
        UpdateSize(camera->Width, camera->Height);
        camera->Start();
#endif /* USE_CAMERA */
    }
    System::Void button3_Click(System::Object^  sender, System::EventArgs^  e) 
    {
        if (video != nullptr) {
            RawImage^ img = video->SnapShot();
            Drawing::Bitmap^ bmp = img->ToBMP();
            bmp->Save("Tmp.png");
            delete bmp;
        }
#ifdef USE_CAMERA
        else if (camera != nullptr) {
            Drawing::Bitmap^ bmp = camera->SnapShot()->ToBMP();
            bmp->Save("Tmp.png");
            delete bmp;
        }
#endif /* USE_CAMERA */

    }
private: System::Void Form1_Shown(System::Object^  sender, System::EventArgs^  e) {

         }
};
}

