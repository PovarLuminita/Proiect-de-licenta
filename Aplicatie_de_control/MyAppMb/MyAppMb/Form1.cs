using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using EasyModbus;
using System.Windows.Forms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;
using System.Data.SqlTypes;

namespace MyAppMb
{
    public partial class Form1 : Form
    {
        EasyModbus.ModbusServer easyModbusTCPServer;
        ModbusClient ModClient; // declaratie obiect
        int[] internal_status = new int[16] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  // array pentru managementul intern al valorilor de pwm
        int[] serverResponse_read_status = new int[16];  // citeste valorile pwm si modifica afisarea led-urilor
        bool[] serverResponseCoils = new bool[16];
        int min = 0; // adresa de start
        int max = 0;  // nr registrii
        private AnimatedLed[] animatedLeds; // array de obiecte pentru cele 16 led-uri

        public Form1()
        {
            InitializeComponent();
            easyModbusTCPServer = new EasyModbus.ModbusServer();
            easyModbusTCPServer.Port = 502; //setez portul 502
            easyModbusTCPServer.UnitIdentifier = 20; //id pentru server
            easyModbusTCPServer.Listen(); ///asculta pe portul 502 default
            SetInitialRegisterandCoilsValues();
            InitializeLeds();
        }

        //initializez valorile din registrii si coils
        private void SetInitialRegisterandCoilsValues()
        {
            for (int i = 0; i < 16; i++)
            {
                easyModbusTCPServer.holdingRegisters[i + 1] = 0;
            }

            for (int i = 0; i < 16; i++)
            {
                bool value = false;
                easyModbusTCPServer.coils[i + 1] = value;
            }
        }

        ///opreste conexiunea cand se inchide aplicatia
        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            base.OnFormClosing(e);

            if (e.CloseReason == CloseReason.UserClosing)
            {
                StopServer();
            }
        }

        private void StopServer()
        {
            if (easyModbusTCPServer != null)
            {
                easyModbusTCPServer.StopListening();
                easyModbusTCPServer = null;
            }
        }


        //incarc valorile in registrii dupa ce citesc de la noduri
        private void UpdateRegisters()
        {
            for (int i = 0; i < 16; i++)
            {
                easyModbusTCPServer.holdingRegisters[i + 1] = (short)serverResponse_read_status[i];
            }
        }

        //incarc starea bobinelor dupa ce citesc de la noduri
        private void UpdateCoils()
        {
            for (int i = 0; i < 16; i++)
            {
                easyModbusTCPServer.coils[i + 1] = serverResponseCoils[i];
            }
        }

        /// <summary>
        /// Functie care initializeaza un array de obiecte pentru a modifica aspectul led-urilor din aplicatie
        /// </summary>
        private void InitializeLeds()
        {
            animatedLeds = new AnimatedLed[16];

            animatedLeds[0] = a1;
            animatedLeds[1] = a2;
            animatedLeds[2] = a3;
            animatedLeds[3] = a4;
            animatedLeds[4] = a5;
            animatedLeds[5] = a6;
            animatedLeds[6] = a7;
            animatedLeds[7] = a8;
            animatedLeds[8] = a9;
            animatedLeds[9] = a10;
            animatedLeds[10] = a11;
            animatedLeds[11] = a12;
            animatedLeds[12] = a13;
            animatedLeds[13] = a14;
            animatedLeds[14] = a15;
            animatedLeds[15] = a16;
        }


        // Functie care face update la culorile de la led-uri in functie de comanda de citire primita de la gateway 
        private void UpdateLedColors()
        {
            for (int i = 0; i < animatedLeds.Length; i++)
            {
                int value = serverResponse_read_status[i];
                animatedLeds[i].BackColor = GetColorFromValue(value);
                animatedLeds[i].Invalidate();  // pentru a afisa schimbarile din nou
            }
        }

        // Functie care alege intensitatea culorii pentru fiecare led in functie de fiecare valoare a pwm-ului
        private Color GetColorFromValue(int value)
        {
            if (value >= 1 && value <= 20)
                return Color.FromArgb(255, 192, 192);
            else if (value >= 21 && value <= 40)
                return Color.FromArgb(255, 128, 128);
            else if (value >= 41 && value <= 60)
                return Color.Red;
            else if (value >= 61 && value <= 80)
                return Color.FromArgb(192, 0, 0);
            else if (value >= 81 && value < 100)
                return Color.Maroon;
            else
                return Color.Silver;
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        // buton pentru conectarea pe seriala
        private void btnConnect_Click(object sender, EventArgs e)
        {
            ModClient = new ModbusClient(txtPort.Text);
            ModClient.Baudrate = 9600;
            ModClient.Parity = System.IO.Ports.Parity.None;
            ModClient.StopBits = System.IO.Ports.StopBits.One;

            try
            {
                ModClient.Connect();
                lblStatus.Text = "CONNECTED";
                btnConnect.Enabled = false;
                btnDisconnect.Enabled = true;
                TogglePWM.Enabled = true;
                ToggleOnOff.Enabled = true;
            }
            catch (Exception ex)
            {
                lblStatus.Text = "ERROR";
            }
        }

        // buton pentru deconectarea pe seriala
        private void btnDisconnect_Click(object sender, EventArgs e)
        {
            ModClient.Disconnect();
            lblStatus.Text = "DISCONNECTED";
            btnConnect.Enabled = true;
            btnDisconnect.Enabled = false;
            TogglePWM.Checked = false;
            ToggleOnOff.Checked = false;
        }

        // buton care trimite comanda pentru ajustare automata
        private void toggleReglareAutomata_CheckedChanged(object sender, EventArgs e)
        {
            if (toggleReglareAutomata.Checked)
            {
                try
                {
                    ModClient.UnitIdentifier = 3;
                    int startingAddress = 5;
                    int registerToSend = 16;

                    ModClient.WriteSingleRegister(startingAddress, registerToSend);
                }
                catch (Exception exc)
                {
                    MessageBox.Show(exc.Message, "Exception writing values to Server", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            else
            {
                try
                {
                    ModClient.UnitIdentifier = 3;
                    int startingAddress = 5;
                    int registerToSend = 15;

                    ModClient.WriteSingleRegister(startingAddress, registerToSend);
                }
                catch (Exception exc)
                {
                    MessageBox.Show(exc.Message, "Exception writing values to Server", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        // buton care verifica daca este comanda pwm on
        private void TogglePWM_CheckedChanged(object sender, EventArgs e)
        {
            if (TogglePWM.Checked)
            {
                ToggleOnOff.Checked = false;
                button_read_sensor.Enabled = true;
                button_set_all_pwm.Enabled = true;
                button_set_pwm.Enabled = true;
                button_send.Enabled = true;
                button_reset_status_pwm.Enabled = true;
                button_read_status_pwm.Enabled = true;
                toggleReglareAutomata.Enabled = true;
                comboBox1.Enabled = true;
                track_pwm.Enabled = true;

                try
                {
                    ModClient.UnitIdentifier = 1;
                    int startingAddress = 5;
                    int registerToSend = 1;

                    ModClient.WriteSingleRegister(startingAddress, registerToSend);
                }
                catch (Exception exc)
                {
                    MessageBox.Show(exc.Message, "Exception writing values to Server", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            else
            {

                toggleReglareAutomata.Enabled = false;

                button_set_all_pwm.Enabled = false;
                button_set_pwm.Enabled = false;
                button_send.Enabled = false;
                button_reset_status_pwm.Enabled = false;
                button_read_status_pwm.Enabled = false;
                comboBox1.Enabled = false;
                track_pwm.Enabled = false;
                button_read_sensor.Enabled = false;

            }
        }

        // buton care verifica daca slave-ul cu led-uri primeste comanda de on/off 
        private void ToggleOnOff_CheckedChanged(object sender, EventArgs e)
        {
            if (ToggleOnOff.Checked)
            {

                button1.Enabled = true;
                toggleButton1.Enabled = true;
                toggleButton2.Enabled = true;
                toggleButton3.Enabled = true;
                toggleButton4.Enabled = true;
                toggleButton5.Enabled = true;
                toggleButton6.Enabled = true;
                toggleButton7.Enabled = true;
                toggleButton8.Enabled = true;
                toggleButton9.Enabled = true;
                toggleButton10.Enabled = true;
                toggleButton11.Enabled = true;
                toggleButton12.Enabled = true;
                toggleButton13.Enabled = true;
                toggleButton14.Enabled = true;
                toggleButton15.Enabled = true;
                toggleButton16.Enabled = true;

                button_ALL_OFF.Enabled = true;
                button_ALL_ON.Enabled = true;
                button_read_status.Enabled = true;
                button_reset_status.Enabled = true;

                TogglePWM.Checked = false;
                toggleReglareAutomata.Enabled = false;

                try
                {
                    ModClient.UnitIdentifier = 1;
                    int startingAddress = 5;
                    int registerToSend = 3; // orice valoare diferita de 1 va comuta pe modul coils

                    ModClient.WriteSingleRegister(startingAddress, registerToSend);
                }
                catch (Exception exc)
                {
                    MessageBox.Show(exc.Message, "Exception writing values to Server", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            else
            {
                TogglePWM.Enabled = true;
                toggleReglareAutomata.Enabled = false;

                toggleButton1.Enabled = false;
                toggleButton2.Enabled = false;
                toggleButton3.Enabled = false;
                toggleButton4.Enabled = false;
                toggleButton5.Enabled = false;
                toggleButton6.Enabled = false;
                toggleButton7.Enabled = false;
                toggleButton8.Enabled = false;
                toggleButton9.Enabled = false;
                toggleButton10.Enabled = false;
                toggleButton11.Enabled = false;
                toggleButton12.Enabled = false;
                toggleButton13.Enabled = false;
                toggleButton14.Enabled = false;
                toggleButton15.Enabled = false;
                toggleButton16.Enabled = false;

                button1.Enabled = false;
                button_ALL_OFF.Enabled = false;
                button_ALL_ON.Enabled = false;
                button_read_status.Enabled = false;
                button_reset_status.Enabled = false;
            }
        }

        // buton care trimite comenzi de scris pe led-uri on/off
        private void button1_Click(object sender, EventArgs e)
        {
            ModClient.UnitIdentifier = 1;

            int startingAddress = 0;
            try
            {
                bool[] coilsToSend = new bool[16];

                coilsToSend[0] = toggleButton1.Checked;
                coilsToSend[1] = toggleButton2.Checked;
                coilsToSend[2] = toggleButton3.Checked;
                coilsToSend[3] = toggleButton4.Checked;
                coilsToSend[4] = toggleButton5.Checked;
                coilsToSend[5] = toggleButton6.Checked;
                coilsToSend[6] = toggleButton7.Checked;
                coilsToSend[7] = toggleButton8.Checked;
                coilsToSend[8] = toggleButton9.Checked;
                coilsToSend[9] = toggleButton10.Checked;
                coilsToSend[10] = toggleButton11.Checked;
                coilsToSend[11] = toggleButton12.Checked;
                coilsToSend[12] = toggleButton13.Checked;
                coilsToSend[13] = toggleButton14.Checked;
                coilsToSend[14] = toggleButton15.Checked;
                coilsToSend[15] = toggleButton16.Checked;

                ModClient.WriteMultipleCoils(startingAddress, coilsToSend);
            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message, "Exception writing values to Server", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        // buton care seteaza toate switch-urile pe modul on
        private void button_ALL_OFF_Click(object sender, EventArgs e)
        {
            toggleButton1.Checked = false;
            toggleButton2.Checked = false;
            toggleButton3.Checked = false;
            toggleButton4.Checked = false;
            toggleButton5.Checked = false;
            toggleButton6.Checked = false;
            toggleButton7.Checked = false;
            toggleButton8.Checked = false;
            toggleButton9.Checked = false;
            toggleButton10.Checked = false;
            toggleButton11.Checked = false;
            toggleButton12.Checked = false;
            toggleButton13.Checked = false;
            toggleButton14.Checked = false;
            toggleButton15.Checked = false;
            toggleButton16.Checked = false;
        }

        // buton care seteaza toate switch-urile pe modul off
        private void button_ALL_ON_Click(object sender, EventArgs e)
        {
            toggleButton1.Checked = true;
            toggleButton2.Checked = true;
            toggleButton3.Checked = true;
            toggleButton4.Checked = true;
            toggleButton5.Checked = true;
            toggleButton6.Checked = true;
            toggleButton7.Checked = true;
            toggleButton8.Checked = true;
            toggleButton9.Checked = true;
            toggleButton10.Checked = true;
            toggleButton11.Checked = true;
            toggleButton12.Checked = true;
            toggleButton13.Checked = true;
            toggleButton14.Checked = true;
            toggleButton15.Checked = true;
            toggleButton16.Checked = true;
        }

        // buton care reseteaza switch-urile
        private void button_reset_status_Click(object sender, EventArgs e)
        {
            animatedLed1.BackColor = Color.Silver;
            animatedLed2.BackColor = Color.Silver;
            animatedLed3.BackColor = Color.Silver;
            animatedLed4.BackColor = Color.Silver;
            animatedLed5.BackColor = Color.Silver;
            animatedLed6.BackColor = Color.Silver;
            animatedLed7.BackColor = Color.Silver;
            animatedLed8.BackColor = Color.Silver;
            animatedLed9.BackColor = Color.Silver;
            animatedLed10.BackColor = Color.Silver;
            animatedLed11.BackColor = Color.Silver;
            animatedLed12.BackColor = Color.Silver;
            animatedLed13.BackColor = Color.Silver;
            animatedLed14.BackColor = Color.Silver;
            animatedLed15.BackColor = Color.Silver;
            animatedLed16.BackColor = Color.Silver;
        }


        // buton care trimite comanda de citit pentru modul coils
        private void button_read_status_Click(object sender, EventArgs e)
        {
            ModClient.UnitIdentifier = 1;
            int startingAddress = 0;
            int txtNumberOfValuesInput = 16;
            try
            {
                serverResponseCoils  = ModClient.ReadCoils(startingAddress, txtNumberOfValuesInput);

                if (serverResponseCoils[0] == true) { animatedLed1.BackColor = Color.DarkRed; } else { animatedLed1.BackColor = Color.Silver; }
                if (serverResponseCoils[1] == true) { animatedLed2.BackColor = Color.DarkRed; } else { animatedLed2.BackColor = Color.Silver; }
                if (serverResponseCoils[2] == true) { animatedLed3.BackColor = Color.DarkRed; } else { animatedLed3.BackColor = Color.Silver; }
                if (serverResponseCoils[3] == true) { animatedLed4.BackColor = Color.DarkRed; } else { animatedLed4.BackColor = Color.Silver; }
                if (serverResponseCoils[4] == true) { animatedLed5.BackColor = Color.DarkRed; } else { animatedLed5.BackColor = Color.Silver; }
                if (serverResponseCoils[5] == true) { animatedLed6.BackColor = Color.DarkRed; } else { animatedLed6.BackColor = Color.Silver; }
                if (serverResponseCoils[6] == true) { animatedLed7.BackColor = Color.DarkRed; } else { animatedLed7.BackColor = Color.Silver; }
                if (serverResponseCoils[7] == true) { animatedLed8.BackColor = Color.DarkRed; } else { animatedLed8.BackColor = Color.Silver; }
                if (serverResponseCoils[8] == true) { animatedLed9.BackColor = Color.DarkRed; } else { animatedLed9.BackColor = Color.Silver; }
                if (serverResponseCoils[9] == true) { animatedLed10.BackColor = Color.DarkRed; } else { animatedLed10.BackColor = Color.Silver; }
                if (serverResponseCoils[10] == true) { animatedLed11.BackColor = Color.DarkRed; } else { animatedLed11.BackColor = Color.Silver; }
                if (serverResponseCoils[11] == true) { animatedLed12.BackColor = Color.DarkRed; } else { animatedLed12.BackColor = Color.Silver; }
                if (serverResponseCoils[12] == true) { animatedLed13.BackColor = Color.DarkRed; } else { animatedLed13.BackColor = Color.Silver; }
                if (serverResponseCoils[13] == true) { animatedLed14.BackColor = Color.DarkRed; } else { animatedLed14.BackColor = Color.Silver; }
                if (serverResponseCoils[14] == true) { animatedLed15.BackColor = Color.DarkRed; } else { animatedLed15.BackColor = Color.Silver; }
                if (serverResponseCoils[15] == true) { animatedLed16.BackColor = Color.DarkRed; } else { animatedLed16.BackColor = Color.Silver; }
                UpdateCoils();
            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message, "Exception Reading values from Server", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        // bara pentru setat pwm
        private void track_pwm_ValueChanged(object sender, EventArgs e)
        {
            value_pwm.Text = track_pwm.Value.ToString();

            if (track_pwm.Value < 30)
            {
                value_pwm.ForeColor = Color.Orange;
                value_pwm.BackColor = Color.LightYellow;
            }

            if (track_pwm.Value > 30 && track_pwm.Value < 70)
            {
                value_pwm.ForeColor = Color.DarkGreen;
                value_pwm.BackColor = Color.DarkSeaGreen;
            }

            if (track_pwm.Value > 50 && track_pwm.Value < 100)
            {
                value_pwm.ForeColor = Color.DarkRed;
                value_pwm.BackColor = Color.OrangeRed;
            }
        }

        // buton care seteaza pwm pentru un singur led
        private void button_set_pwm_Click(object sender, EventArgs e)
        {

            bool flag_internal_status_gol = false;

            for (int i = 0; i < 16; i++)
            {
                if (internal_status[i] == 0)
                    flag_internal_status_gol = true;
            }

            if (flag_internal_status_gol)
            {

                ModClient.UnitIdentifier = 1;
                int startingAddress = 6;
                int txtNumberOfValuesInput = 16;

                try
                {
                    int[] serverResponse = new int[16];

                    serverResponse = ModClient.ReadHoldingRegisters(startingAddress, txtNumberOfValuesInput);

                    serverResponse.CopyTo(internal_status, 0);  // copiaza continutul din serverResonse in internal_status de la indexul 0
                    flag_internal_status_gol = false;

                }
                catch (Exception exc)
                {
                    MessageBox.Show(exc.Message, "Exception Reading values from Server", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }

            int index_set_led = 0;

            if (comboBox1.SelectedIndex == 0)
            { text1.Text = track_pwm.Value.ToString(); index_set_led = 0; internal_status[0] = track_pwm.Value; }

            if (comboBox1.SelectedIndex == 1)
            { text2.Text = track_pwm.Value.ToString(); index_set_led = 1; internal_status[1] = track_pwm.Value; }

            if (comboBox1.SelectedIndex == 2)
            { text3.Text = track_pwm.Value.ToString(); index_set_led = 2; internal_status[2] = track_pwm.Value; }

            if (comboBox1.SelectedIndex == 3)
            { text4.Text = track_pwm.Value.ToString(); index_set_led = 3; internal_status[3] = track_pwm.Value; }

            if (comboBox1.SelectedIndex == 4)
            { text5.Text = track_pwm.Value.ToString(); index_set_led = 4; internal_status[4] = track_pwm.Value; }

            if (comboBox1.SelectedIndex == 5)
            { text6.Text = track_pwm.Value.ToString(); index_set_led = 5; internal_status[5] = track_pwm.Value; }

            if (comboBox1.SelectedIndex == 6)
            { text7.Text = track_pwm.Value.ToString(); index_set_led = 6; internal_status[6] = track_pwm.Value; }

            if (comboBox1.SelectedIndex == 7)
            { text8.Text = track_pwm.Value.ToString(); index_set_led = 7; internal_status[7] = track_pwm.Value; }

            if (comboBox1.SelectedIndex == 8)
            { text9.Text = track_pwm.Value.ToString(); index_set_led = 8; internal_status[8] = track_pwm.Value; }

            if (comboBox1.SelectedIndex == 9)
            { text10.Text = track_pwm.Value.ToString(); index_set_led = 9; internal_status[9] = track_pwm.Value; }

            if (comboBox1.SelectedIndex == 10)
            { text11.Text = track_pwm.Value.ToString(); index_set_led = 10; internal_status[10] = track_pwm.Value; }

            if (comboBox1.SelectedIndex == 11)
            { text12.Text = track_pwm.Value.ToString(); index_set_led = 11; internal_status[11] = track_pwm.Value; }

            if (comboBox1.SelectedIndex == 12)
            { text13.Text = track_pwm.Value.ToString(); index_set_led = 12; internal_status[12] = track_pwm.Value; }

            if (comboBox1.SelectedIndex == 13)
            { text14.Text = track_pwm.Value.ToString(); index_set_led = 13; internal_status[13] = track_pwm.Value; }

            if (comboBox1.SelectedIndex == 14)
            { text15.Text = track_pwm.Value.ToString(); index_set_led = 14; internal_status[14] = track_pwm.Value; }

            if (comboBox1.SelectedIndex == 15)
            { text16.Text = track_pwm.Value.ToString(); index_set_led = 15; internal_status[15] = track_pwm.Value; }


            if (index_set_led > max)
                max = index_set_led;

            if (index_set_led < min)
                min = index_set_led;
        }

        // buton care trimite comanda de scris pwm pe toate led-urile in functie de valoarea selectata din trackbar
        private void button_set_all_pwm_Click(object sender, EventArgs e)
        {
            int value_all_pwm = track_pwm.Value;

            if (value_all_pwm >= 1 && value_all_pwm < 100)
            {
                text1.Text = track_pwm.Value.ToString();
                text2.Text = track_pwm.Value.ToString();
                text3.Text = track_pwm.Value.ToString();
                text4.Text = track_pwm.Value.ToString();
                text5.Text = track_pwm.Value.ToString();
                text6.Text = track_pwm.Value.ToString();
                text7.Text = track_pwm.Value.ToString();
                text8.Text = track_pwm.Value.ToString();
                text9.Text = track_pwm.Value.ToString();
                text10.Text = track_pwm.Value.ToString();
                text11.Text = track_pwm.Value.ToString();
                text12.Text = track_pwm.Value.ToString();
                text13.Text = track_pwm.Value.ToString();
                text14.Text = track_pwm.Value.ToString();
                text15.Text = track_pwm.Value.ToString();
                text16.Text = track_pwm.Value.ToString();
            }

            
            ModClient.UnitIdentifier = 1;
            int txtStartingAddressOutput = 6;

            try
            {

                int[] registersToSend = new int[16];


                for (int i = 0; i < 16; i++)
                {
                    registersToSend[i] = track_pwm.Value; // adauga valoarea setata cu bara pentru pwm
                }

                ModClient.WriteMultipleRegisters(txtStartingAddressOutput, registersToSend);
                Array.Clear(internal_status, 0, 16); // reseteaza valorile din internal status
                min = 0;
                max = 0;
            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message, "Exception writing values to Server", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

        }

        // buton pentru a trimite comanda de scris pwm 
        private void button_send_Click(object sender, EventArgs e)
        {
            ModClient.UnitIdentifier = 1;
            int txtStartingAddressOutput = 6 + min;
            int calcul_nr_registers = 0;

            calcul_nr_registers = max - min + 1;

            try
            {

                int[] registersToSend = new int[calcul_nr_registers];

                int i = 0;

                for (int j = min; j <= max; j++)
                {
                    registersToSend[i] = internal_status[j];
                    i++;
                }

                ModClient.WriteMultipleRegisters(txtStartingAddressOutput, registersToSend);

                Array.Clear(internal_status, 0, 16);  
                min = 0;
                max = 0;

            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message, "Exception writing values to Server", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        // buton care trimite comanda de citit pwm
        private void button_read_status_pwm_Click(object sender, EventArgs e)
        {

            ModClient.UnitIdentifier = 1;
            int startingAddress = 6;
            int txtNumberOfValuesInput = 16;

            try
            {
                serverResponse_read_status = ModClient.ReadHoldingRegisters(startingAddress, txtNumberOfValuesInput);

                UpdateLedColors(); // functie care face update la culorile led-urilor din aplicatie
                UpdateRegisters();


            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message, "Exception Reading values from Server", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        // buton care reseteaza afisarea led-urilor din aplicatie
        private void button_reset_status_pwm_Click(object sender, EventArgs e)
        {
            a1.BackColor = Color.Silver;
            a2.BackColor = Color.Silver;
            a3.BackColor = Color.Silver;
            a4.BackColor = Color.Silver;
            a5.BackColor = Color.Silver;
            a6.BackColor = Color.Silver;
            a7.BackColor = Color.Silver;
            a8.BackColor = Color.Silver;
            a9.BackColor = Color.Silver;
            a10.BackColor = Color.Silver;
            a11.BackColor = Color.Silver;
            a12.BackColor = Color.Silver;
            a13.BackColor = Color.Silver;
            a14.BackColor = Color.Silver;
            a15.BackColor = Color.Silver;
            a16.BackColor = Color.Silver;
        }

        // buton care trimite comanda de citit la slave-ul cu senzorul
        private void button_read_sensor_Click(object sender, EventArgs e)
        {

            ModClient.UnitIdentifier = 5;
            int txtStartingAddressInput = 5;
            int txtNumberOfValuesInput = 1;

            try
            {
                int[] serverResponse = ModClient.ReadHoldingRegisters(txtStartingAddressInput, txtNumberOfValuesInput);

                value_sensor.Text = serverResponse[0].ToString();

                if (serverResponse[0] <= 100)
                    progressBar1.Value = 20;

                if (serverResponse[0] > 100 && serverResponse[0] <= 200)
                    progressBar1.Value = 40;

                if (serverResponse[0] > 200 && serverResponse[0] <= 300)
                    progressBar1.Value = 60;

                if (serverResponse[0] > 300 && serverResponse[0] <= 400)
                    progressBar1.Value = 80;

                if (serverResponse[0] > 400 && serverResponse[0] <= 500)
                    progressBar1.Value = 100;

                if (serverResponse[0] > 500 && serverResponse[0] <= 600)
                    progressBar1.Value = 120;

                if (serverResponse[0] > 600)
                    progressBar1.Value = 140;

            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message, "Exception Reading values from Server", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }


    }
}