REM Aydan Wynos
REM
REM This program will allow the user to connect to, control and collect data from an Agilent Technologies
REM Oscilloscope using multiple channels. The program will print data to a text file along with a timestamp for each
REM data point.

Imports System.IO
Imports Ivi.Visa.Interop

Public Class FRM1

    Private myMgr As ResourceManager
    Private myScope As FormattedIO488
    Private scopeData As StreamWriter
    Private startTime As Double, timeStamp As Double
    Private temp As Double
    Private varQueryResult As Byte()
    Private chanNum As String

    Private Sub getTime()

        startTime = DateTime.Now.ToOADate

    End Sub

    Private Sub timeConversion()

        temp = Math.Truncate(startTime)

        startTime = startTime - temp    'Subtracts away the integer, leaving just the numbers after the decimal

        'There are 24 hours in a day, 60 minutes in an hour and 60 seconds in a day. To get the seconds, multiply the fractional days
        'by 24, 60 and 60.
        startTime = startTime * 24 * 60 * 60

    End Sub

    Private Sub FRM1_Load(sender As Object, e As EventArgs) Handles MyBase.Load

        myMgr = New ResourceManager
        myScope = New FormattedIO488

        myScope.IO = myMgr.Open("USB0::2391::5965::MY50340366::INSTR")

        scopeData = File.CreateText("C:\Users\AWynos\Desktop\Oscilloscope Program\ScopeData.txt")

    End Sub

    Private Sub BTN1_Click(sender As Object, e As EventArgs) Handles BTN1.Click

        myScope.WriteString(":Timebase:Mode Main")
        myScope.WriteString(":Acquire:Type Normal")
        myScope.WriteString(":Acquire:Complete 100")

        REM No ":Waveform:Source" command is required. The ":Digitize" command without specified channels will
        REM Digitize all channels that are currently displayed. The initialize button will take care of which channels
        REM are to be displayed.
        myScope.WriteString(":Waveform:Format BYTE")
        myScope.WriteString(":Waveform:Points MAXimum")

        getTime()
        myScope.WriteString(":Digitize")
        timeConversion()

    End Sub

    Private Sub BTN2_Click(sender As Object, e As EventArgs) Handles BTN2.Click

        Dim Preamble()
        Dim intFormat As Integer
        Dim intType As Integer
        Dim lngPoints As Long
        Dim lngCount As Long
        Dim dblXIncrement As Double
        Dim dblXOrigin As Double
        Dim lngXReference As Long
        Dim sngYIncrement As Single
        Dim sngYOrigin As Single
        Dim lngYReference As Long
        Dim strOutput As String
        Dim timeRef As Double
        Dim timeOffset As Double

        'myScope.WriteString(":WAVEFORM:SOURCE CHAN" + chanNum) 'sets source to channel 1

        myScope.WriteString(":WAVEFORM:POINTS:MODE Normal")
        myScope.WriteString(":WAVEFORM:POINTS MAXimum") 'sets points to MAX

        Dim lngVSteps As Long
        Dim intBytesPerData As Integer

        myScope.WriteString(":WAVEFORM:FORMAT BYTE") 'sets data format to BYTE
        lngVSteps = 65536
        intBytesPerData = 1

        myScope.WriteString(":WAVEFORM:PREAMBLE?") 'Query for the preamble.

        Preamble = myScope.ReadList     'Reads preamble information and places it into an array
        intFormat = Preamble(0)         'Format: 0 = Byte, 1 = Word, 4 = ASCII
        intType = Preamble(1)           'Type: 0 = Normal, 1 = Peak, 2 = Average
        lngPoints = Preamble(2)         'Points: Number of data points transfered
        lngCount = Preamble(3)          'Will always be 1
        dblXIncrement = Preamble(4)     'Time difference between data points
        dblXOrigin = Preamble(5)        'Always the first data point in memory
        lngXReference = Preamble(6)     'Specifies the data point associated with X-Origin
        sngYIncrement = Preamble(7)     'Voltage difference between data points
        sngYOrigin = Preamble(8)        'Value is the voltage at center screen
        lngYReference = Preamble(9)     'Specifies the data point where y-origin occurs
        strOutput = ""                  'Variable holding final string output

        myScope.WriteString(":WAV:DATA?") 'Query the scope for waveform data

        Dim lngI As Long
        Dim lngDataValue As Long

        timeOffset = (dblXOrigin - lngXReference * dblXIncrement)
        scopeData.WriteLine("Waveform data:" + vbCrLf + strOutput) 'Writes Preamble information to scopeData
        scopeData.WriteLine()
        varQueryResult = myScope.ReadIEEEBlock(IEEEBinaryType.BinaryType_UI1) 'Reads buffered waveform data from the scope in a binary block
        scopeData.WriteLine("Volts, Time (seconds)")
        For lngI = 0 To UBound(varQueryResult)
            If (lngI Mod 1) = 0 Then
                strOutput = "" 'Resets strOutput to clear memory
                lngDataValue = varQueryResult(lngI) ' 8-bit value
                strOutput += FormatNumber((lngDataValue - lngYReference) * sngYIncrement + sngYOrigin, 5) + "," 'Shows voltage
                REM Get the time reference from the scope
                timeRef = ((lngI / intBytesPerData - lngXReference) * dblXIncrement + dblXOrigin)
                timeStamp = timeRef - timeOffset + startTime
                strOutput += Format(timeStamp, "#####.#########")
                scopeData.WriteLine(strOutput) 'Writes data from current loop iteration to the data file
            End If
        Next lngI

        scopeData.Close()

    End Sub

    Private Sub BTN3_Click(sender As Object, e As EventArgs) Handles BTN3.Click

        myScope.IO.Clear()

        'myScope.WriteString("*RST")

        myScope.WriteString(":Timebase:Range 10")
        myScope.WriteString(":Timebase:Delay 0")
        myScope.WriteString(":Timebase:Reference Center")

        'myScope.WriteString(":Channel" + chanNum + ":Probe 10")
        'myScope.WriteString(":Channel" + chanNum + ":Range 8")
        'myScope.WriteString(":Channel" + chanNum + ":Offset 0")
        'myScope.WriteString(":Channel" + chanNum + ":Coupling DC")

        myScope.WriteString(":Acquire:Type Normal")

        If CHK1.Checked = True Then
            myScope.WriteString(":Channel1:Display ON")
        Else
            myScope.WriteString(":Channel1:Display OFF")
        End If

        If CHK2.Checked = True Then
            myScope.WriteString(":Channel2:Display ON")
        Else
            myScope.WriteString(":Channel2:Display OFF")
        End If

        If CHK3.Checked = True Then
            myScope.WriteString(":Channel3:Display ON")
        Else
            myScope.WriteString(":Channel3:Display OFF")
        End If

        If CHK4.Checked = True Then
            myScope.WriteString(":Channel4:Display ON")
        Else
            myScope.WriteString(":Channel4:Display OFF")
        End If

    End Sub

    Private Sub BTN5_Click(sender As Object, e As EventArgs) Handles BTN5.Click

        Dim rng As String, range As String

        rng = TXT2.Text * 10

        range = ":Timebase:Range " + rng

        myScope.WriteString(range)

    End Sub

    Private Sub BTN4_Click(sender As Object, e As EventArgs) Handles BTN4.Click

        Dim chanRng As Double, chanRange As String
        Dim c As Integer, m As Integer

        If CHK1.Checked = True Then
            chanNum = 1
            c += 1
        End If

        If CHK2.Checked = True Then
            chanNum = 2
            c += 1
        End If

        If CHK3.Checked = True Then
            chanNum = 3
            c += 1
        End If

        If CHK4.Checked = True Then
            chanNum = 4
            c += 1
        End If

        If c > 1 Then
            m = MessageBox.Show("Please select only one channel!", "Error", _
                                MessageBoxButtons.OK, MessageBoxIcon.Warning)
            If m = 1 Then
                TXT1.Clear()
                TXT1.Focus()
                CHK1.Checked = False
                CHK2.Checked = False
                CHK3.Checked = False
                CHK4.Checked = False
            End If
        End If

        If c < 1 Then
            m = MessageBox.Show("Please select a channel", "Error", _
                                MessageBoxButtons.OK, MessageBoxIcon.Warning)
            If m = 1 Then
                TXT1.Clear()
                TXT1.Focus()
            End If
        End If

        chanRng = TXT1.Text * 8

        chanRange = ":Channel" + chanNum + ":Range " + chanRng.ToString

        myScope.WriteString(chanRange)

    End Sub

End Class
