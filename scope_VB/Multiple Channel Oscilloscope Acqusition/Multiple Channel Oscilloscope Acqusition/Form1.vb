﻿REM Aydan Wynos
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

    Private Sub setChan1()

        Dim chanRange As String

        chanRange = TXT1.Text * 8

        myScope.WriteString(":Channel1:Range " + chanRange)

    End Sub

    Private Sub setChan2()

        Dim chanrange As String

        chanrange = TXT1.Text * 8

        myScope.WriteString(":Channel2:Range " + chanrange)

    End Sub

    Private Sub setChan3()

        Dim chanRange As String

        chanRange = TXT1.Text * 8

        myScope.WriteString(":Channel3:Range " + chanRange)

    End Sub

    Private Sub setChan4()

        Dim chanRange As String

        chanRange = TXT1.Text * 8

        myScope.WriteString(":Channel4:Range " + chanRange)

    End Sub

    Private Sub FRM1_Load(sender As Object, e As EventArgs) Handles MyBase.Load

        myMgr = New ResourceManager
        myScope = New FormattedIO488

        myScope.IO = myMgr.Open("USB0::2391::5965::MY50340366::INSTR")

        'scopeData = File.CreateText("C:\Users\AWynos\Desktop\Oscilloscope Program\ScopeData.txt")

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
        Dim Channel(4)
        Dim filePath As String
        Dim i As Integer

        For i = 1 To 4
            Channel(i) = -1
        Next

        If CHK1.Checked = True Then
            Channel(1) = 1
        End If
        If CHK2.Checked = True Then
            Channel(2) = 2
        End If
        If CHK3.Checked = True Then
            Channel(3) = 3
        End If
        If CHK4.Checked = True Then
            Channel(4) = 4
        End If

        For i = 1 To 4

            If Channel(i) <> -1 Then

                filePath = "C:\Users\AWynos\Desktop\Oscilloscope Program\Chan" + i.ToString + ".txt"
                scopeData = File.CreateText(filePath) 'Creates a file for each channel being digitized

                myScope.WriteString(":WAVEFORM:SOURCE CHAN" + i.ToString) 'Sets channel source

                myScope.WriteString(":WAVEFORM:POINTS:MODE Normal")
                myScope.WriteString(":WAVEFORM:POINTS MAXimum") 'Sets points to max

                Dim lngVSteps As Long
                Dim intBytesPerData As Integer

                myScope.WriteString(":WAVEFORM:FORMAT BYTE") 'Sets format to bytes
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

                timeOffset = (dblXOrigin - lngXReference * dblXIncrement) 'Finds the time difference between data points
                scopeData.WriteLine("Data from Channel " + i.ToString) 'Writes which channel data is being captured from
                scopeData.WriteLine("Waveform data:" + vbCrLf + strOutput) 'Writes Preamble information to scopeData
                varQueryResult = myScope.ReadIEEEBlock(IEEEBinaryType.BinaryType_UI1) 'Reads buffered waveform data from the scope in a binary block
                scopeData.WriteLine("Volts, Time (seconds)")
                For lngI = 0 To UBound(varQueryResult)
                    If (lngI Mod 1) = 0 Then
                        strOutput = "" 'Resets strOutput to clear memory
                        lngDataValue = varQueryResult(lngI) ' 8-bit value
                        strOutput += FormatNumber((lngDataValue - lngYReference) * sngYIncrement + sngYOrigin, 5) + ", " 'Shows voltage
                        REM Get the time reference from the scope
                        timeRef = ((lngI / intBytesPerData - lngXReference) * dblXIncrement + dblXOrigin)
                        timeStamp = timeRef - timeOffset + startTime 'Creates a timestamp for each data point
                        strOutput += Format(timeStamp, "#####.#########") 'Formats data in the text file
                        scopeData.WriteLine(strOutput) 'Writes data from current loop iteration to the data file
                    End If
                Next lngI

                scopeData.Close()

            End If
        Next i

    End Sub

    Private Sub BTN3_Click(sender As Object, e As EventArgs) Handles BTN3.Click

        myScope.IO.Clear()

        myScope.WriteString(":Timebase:Delay 0")
        myScope.WriteString(":Timebase:Reference Left")
        myScope.WriteString(":Acquire:Type Normal")
        myScope.WriteString(":RUN")

        If CHK1.Checked = True Then
            myScope.WriteString(":Channel1:Display ON")
            myScope.WriteString(":Channel1:Offset 0")
        Else
            myScope.WriteString(":Channel1:Display OFF")
        End If

        If CHK2.Checked = True Then
            myScope.WriteString(":Channel2:Display ON")
            myScope.WriteString(":Channel2:Offset 0")
        Else
            myScope.WriteString(":Channel2:Display OFF")
        End If

        If CHK3.Checked = True Then
            myScope.WriteString(":Channel3:Display ON")
            myScope.WriteString(":Channel3:Offset 0")
        Else
            myScope.WriteString(":Channel3:Display OFF")
        End If

        If CHK4.Checked = True Then
            myScope.WriteString(":Channel4:Display ON")
            myScope.WriteString(":Channel4:Offset 0")
        Else
            myScope.WriteString(":Channel4:Display OFF")
        End If

    End Sub

    Private Sub BTN5_Click(sender As Object, e As EventArgs) Handles BTN5.Click

        Dim rng As String, range As String
        Dim m As Integer

        Try
            rng = TXT2.Text * 10

            range = ":Timebase:Range " + rng

            myScope.WriteString(range)
        Catch ex As Exception
            m = MessageBox.Show("Please enter a timebase range!", "No Range Entered", _
                                MessageBoxButtons.OK, MessageBoxIcon.Warning)
            If m = Windows.Forms.DialogResult.OK Then
                TXT2.Clear()
                TXT2.Focus()
                Exit Sub
            End If
        End Try

    End Sub

    Private Sub BTN4_Click(sender As Object, e As EventArgs) Handles BTN4.Click

        Dim m As Integer

        If CHK1.Checked = False And CHK2.Checked = False And _
            CHK3.Checked = False And CHK4.Checked = False Then
            m = MessageBox.Show("Please select a source channel!", "No Channel Selected", _
                                MessageBoxButtons.OK, MessageBoxIcon.Warning)
        End If

        If CHK1.Checked = True Then
            Try
                setChan1()
            Catch ex As Exception
                m = MessageBox.Show("Please enter a channel range!", "No Range Entered", _
                                    MessageBoxButtons.OK, MessageBoxIcon.Warning)
                If m = Windows.Forms.DialogResult.OK Then
                    TXT1.Clear()
                    TXT1.Focus()
                    Exit Sub
                End If
            End Try
        End If

        If CHK2.Checked = True Then
            Try
                setChan2()
            Catch ex As Exception
                m = MessageBox.Show("Please enter a channel range!", "No Range Entered", _
                                    MessageBoxButtons.OK, MessageBoxIcon.Warning)
                If m = Windows.Forms.DialogResult.OK Then
                    TXT1.Clear()
                    TXT1.Focus()
                    Exit Sub
                End If
            End Try
        End If

        If CHK3.Checked = True Then
            Try
                setChan3()
            Catch ex As Exception
                m = MessageBox.Show("Please enter a channel range!", "No Range Entered", _
                                    MessageBoxButtons.OK, MessageBoxIcon.Warning)
                If m = Windows.Forms.DialogResult.OK Then
                    TXT1.Clear()
                    TXT1.Focus()
                    Exit Sub
                End If
            End Try
        End If

        If CHK4.Checked = True Then
            Try
                setChan4()
            Catch ex As Exception
                m = MessageBox.Show("Please enter a channel range!", "No Range Entered", _
                                    MessageBoxButtons.OK, MessageBoxIcon.Warning)
                If m = Windows.Forms.DialogResult.OK Then
                    TXT1.Clear()
                    TXT1.Focus()
                    Exit Sub
                End If
            End Try
        End If

    End Sub

    Private Sub BTN6_Click(sender As Object, e As EventArgs) Handles BTN6.Click

        'scopeData.Close()

        End

    End Sub

End Class
