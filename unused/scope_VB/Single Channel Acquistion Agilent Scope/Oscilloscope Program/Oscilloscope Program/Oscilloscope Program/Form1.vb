REM Aydan Wynos
REM
REM This program is designed to connect to and read data from a
REM 3000 series InfiniiVision Digital Storage Oscilloscope. It will then send
REM the data to a binary file for analysis by the user.

Imports System.IO
Imports Ivi.Visa.Interop

Public Class FRM1

    Private scopeData As StreamWriter
    Private scopeDataRead As StreamReader
    Private myMgr As ResourceManager
    Private myScope As FormattedIO488
    Private adjust As Double, offset As Double
    Private startTime As Double, timeStamp As Double
    Private temp As Double
    Private lngVSteps As Long, intBytesPerData As Integer
    Private varQueryResult As Byte()


    Private Sub FRM1_Load(sender As Object, e As EventArgs) Handles MyBase.Load

        myMgr = New ResourceManager
        myScope = New FormattedIO488

        myScope.IO = myMgr.Open("USB0::2391::5965::MY50340366::INSTR")

        scopeData = File.CreateText("C:\Users\AWynos\Desktop\Oscilloscope Program\ScopeData.txt")

    End Sub

    Private Sub BTN1_Click(sender As Object, e As EventArgs) Handles BTN1.Click

        myScope.WriteString(":Autoscale")

        offset = 0
        adjust = 0

        TXT2.Clear()
        TXT1.Clear()

    End Sub

    Private Sub BTN3_Click(sender As Object, e As EventArgs) Handles BTN3.Click

        Dim chanRng As String, chanRange As String

        chanRng = TXT2.Text * 8

        chanRange = ":Channel3:Range " + chanRng

        myScope.WriteString(chanRange)

    End Sub

    Private Sub BTN4_Click(sender As Object, e As EventArgs) Handles BTN4.Click

        Dim rng As String, range As String

        rng = TXT1.Text * 10

        range = ":Timebase:Range " + rng

        myScope.WriteString(range)

    End Sub

    Private Sub BTN7_Click(sender As Object, e As EventArgs) Handles BTN7.Click

        Dim change As String, result

        myScope.WriteString(":Timebase:Delay?")

        result = myScope.ReadString

        adjust = result - 0.000125
        change = adjust.ToString

        myScope.WriteString(":Timebase:Delay " + change)

        myScope.WriteString(":Channel3:Display ON")

    End Sub

    Private Sub BTN5_Click(sender As Object, e As EventArgs) Handles BTN5.Click

        Dim change As String, result As String

        myScope.WriteString(":Channel3:Offset?")

        result = myScope.ReadString

        offset = result - 0.1
        change = offset.ToString

        myScope.WriteString(":Channel3:Offset " + change)

    End Sub

    Private Sub BTN6_Click(sender As Object, e As EventArgs) Handles BTN6.Click

        Dim change As String, result As String

        myScope.WriteString(":Channel3:Offset?")

        result = myScope.ReadString

        offset = result + 0.1
        change = offset.ToString

        myScope.WriteString(":Channel3:Offset " + change)

    End Sub

    Private Sub BTN8_Click(sender As Object, e As EventArgs) Handles BTN8.Click

        Dim change As String, result As String

        myScope.WriteString(":Timebase:Delay?")

        result = myScope.ReadString

        adjust = result + 0.000125
        change = adjust.ToString

        myScope.WriteString(":Timebase:Delay " + change)

    End Sub

    Private Sub getTime()

        startTime = DateTime.Now.ToOADate

    End Sub

    Private Sub timeConversion()

        temp = Math.Truncate(startTime)

        'MsgBox(startTime)
        'MsgBox(temp)
        startTime = startTime - temp    'Subtracts away the integer, leaving just the numbers after the decimal
        'MsgBox(startTime)

        'There are 24 hours in a day, 60 minutes in an hour and 60 seconds in a day. 
        startTime = startTime * 24 * 60 * 60

        'MsgBox(startTime)

    End Sub

    Private Sub BTN9_Click(sender As Object, e As EventArgs) Handles BTN9.Click

        myScope.WriteString(":Timebase:Mode Main")
        myScope.WriteString(":Acquire:Type Normal")
        myScope.WriteString(":Acquire:Complete 100")

        myScope.WriteString(":Waveform:Source channel3")
        myScope.WriteString(":Waveform:Format BYTE")
        myScope.WriteString(":Waveform:Points MAXimum")

        getTime()
        myScope.WriteString(":Digitize Channel3")
        timeConversion()

    End Sub

    Private Sub BTN11_Click(sender As Object, e As EventArgs) Handles BTN11.Click

        scopeData.Close()

        End

    End Sub

    Private Sub BTN13_Click(sender As Object, e As EventArgs) Handles BTN13.Click

        'On Error GoTo VisaComError 'Error catching system sends error message to MsgBox

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

        myScope.WriteString(":WAVEFORM:SOURCE CHAN3") 'sets source to channel 1

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

        'strOutput += "Volts/Div = " + FormatNumber(lngVSteps * sngYIncrement / 8) + " V" + vbCrLf
        'strOutput += "Offset = " + FormatNumber(sngYOrigin) + " V" + vbCrLf
        'strOutput += "Sec/Div = " + FormatNumber(lngPoints * dblXIncrement / 10 * 1000000) + " us" + vbCrLf
        'strOutput += "Delay = " + FormatNumber(((lngPoints / 2) * dblXIncrement + dblXOrigin) * 1000000) + " us" + vbCrLf + vbCrLf

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

        'VisaComError:
        'MsgBox("VISA COM Error:" + vbCrLf + Err.Description)
    End Sub

    Private Sub BTN14_Click(sender As Object, e As EventArgs) Handles BTN14.Click

        'myScope.IO.Clear()

        'myScope.WriteString("*RST")

        myScope.WriteString(":Timebase:Range 10")
        myScope.WriteString(":Timebase:Delay 0")
        myScope.WriteString(":Timebase:Reference Center")

        myScope.WriteString(":Channel3:Probe 10")
        myScope.WriteString(":Channel3:Range 8")
        myScope.WriteString(":Channel3:Offset 0")
        myScope.WriteString(":Channel3:Coupling DC")

        myScope.WriteString(":Acquire:Type Normal")

    End Sub

    Private Sub BTN2_Click(sender As Object, e As EventArgs) Handles BTN2.Click

        myScope.WriteString(":Channel1:Display ON")
        myScope.WriteString(":Channel2:Display ON")
        myScope.WriteString(":Channel3:Display ON")
        myScope.WriteString(":RUN")

        'getTime()
        'timeConversion()

    End Sub

End Class
