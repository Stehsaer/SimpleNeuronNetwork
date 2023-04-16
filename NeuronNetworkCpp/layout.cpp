// STL
#include <iostream>
#include <vector>
#include <thread>

#include "uni_imgui.h"
#include "imgui/extensions/imgui_indeterminate_progressbar.h" // indeterminate progressbar extension

#include "UIState.h"
#include "UIStyle.h"
#include "Tasks.h"

// Customized UI Components
#include "NotifyUI.h"
#include "PopupModal.h"

// Helpers
#include "MathHelper.h"
#include "FormatHelper.h"

// Font Definition
#include "fonts/RemixIconFontDef.h"

namespace UI
{
	namespace Layout
	{
		// layout constants
		const int bottomBarHeight = 35;
		const int rightPanelWidth = 400;
		
		// demo flags
		bool showDemoWindow = false;
		bool showPlotDemo = false;

		namespace Util
		{
			namespace FrametimeRecorder
			{
				float frametimeNow;

				std::vector<float> frameTimeList;
				const int maxRecord = 300;
				float frameTimes[maxRecord];

				const ImGui::SimplePlot::PlotConfig frametimeGraphConfig
				{
					.values = ImGui::SimplePlot::PlotConfig::Values
					{
						.ys = &frameTimes[0],
						.count = maxRecord
					},
					.scale = ImGui::SimplePlot::PlotConfig::Scale
					{
						.min = 0.0f, .max = 100.0f
					},
					.tooltip = ImGui::SimplePlot::PlotConfig::Tooltip
					{
						.show = false
					},
					.grid_x = ImGui::SimplePlot::PlotConfig::Grid
					{
						.show = true,
					},
					.grid_y = ImGui::SimplePlot::PlotConfig::Grid
					{
						.show = true,
						.size = 20,
						.subticks = 4
					},
					.frame_size = ImVec2(400,200),
					.line_thickness = 2.0f
				};

				void RecordFPS()
				{
					frametimeNow = ImGui::GetIO().DeltaTime * 1000.0f;
					frameTimeList.push_back(frametimeNow);
					if (frameTimeList.size() > maxRecord) frameTimeList.erase(frameTimeList.begin());
				}

				void DisplayFPSGraph()
				{
					for (int i = 0; i < maxRecord; i++)
					{
						if (i < frameTimeList.size())
							frameTimes[i] = frameTimeList[i];
						else
							frameTimes[i] = 0.0f;
					}

					ImGui::BulletText("Avg Frame time: %.1fms", 1000.0f / ImGui::GetIO().Framerate);

					//ImGui::PlotLines("##frametime_plot", [](void* data, int idx) {return (*(std::vector<float>*) data)[idx]; }, &frameTimeList, frameTimeList.size(), 0, 0, 0.0, FLT_MAX, ImVec2(400, 200));
					ImGui::SimplePlot::Plot("##frametime_plot", frametimeGraphConfig);
				}
			}

			void DatasetCombo(std::string title, int* target)
			{
				auto& datasets = Tasks::Instances::datasets;

				if (datasets.size() > 0)
				{
					if (ImGui::BeginCombo(GenID(title, target).c_str(), datasets[*target]->name.c_str()))
					{
						for (int n = 0; n < datasets.size(); n++)
						{
							const bool selected = (*target == n);
							if (ImGui::Selectable(GenID(std::format("{} (Slot {})", datasets[n]->name, n), datasets[n]).c_str(), selected))
							{
								*target = n;
							}

							if (selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
				}
				else
				{
					ImGui::BulletText("No dataset loaded! :(");
				}
			}

			void ShowHeatmap(Network::NeuronLayer& layer)
			{
				if (layer.Count() == 0)
					return;

				// square?
				if (((int)sqrt(layer.prevCount)) * ((int)sqrt(layer.prevCount)) == layer.prevCount)
				{
					int size = sqrt(layer.prevCount);

					ImPlot::PushColormap(ImPlotColormap_Jet);

					int i = 0;
					int columns = ImGui::GetContentRegionAvail().x / 160;

					for (auto& neuron : layer.neurons)
					{
						try
						{
							if (ImPlot::BeginPlot(GenID("", &neuron).c_str(), ImVec2(120, 120)))
							{
								const ImPlotAxisFlags axes_flags = 
									ImPlotAxisFlags_Lock | 
									ImPlotAxisFlags_NoDecorations | 
									ImPlotAxisFlags_NoMenus | 
									ImPlotAxisFlags_NoSideSwitch|
									ImPlotAxisFlags_NoHighlight;

								ImPlot::SetupAxes(NULL, NULL, axes_flags, axes_flags);

								ImPlot::PlotHeatmap(GenID("", &neuron).c_str(), neuron.weights, size, size, 0.0, 0.0, NULL);

								ImPlot::EndPlot();
							}

							// Detailed view of neuron
							if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
							{
								if (ImGui::BeginTooltip())
								{
									if (ImPlot::BeginPlot(GenID(std::format("Neuron #{}", i), &neuron).c_str(), ImVec2(400, 400)))
									{
										const ImPlotAxisFlags axes_flags = ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_NoMenus | ImPlotAxisFlags_NoSideSwitch;
										ImPlot::SetupAxes(NULL, NULL, axes_flags, axes_flags);

										ImPlot::PlotHeatmap(GenID("", &neuron).c_str(), neuron.weights, size, size, 0.0, 0.0, NULL);

										ImPlot::EndPlot();
									}
									ImGui::EndTooltip();
								}
							}
						}
						catch (std::exception e)
						{
							ImPlot::EndPlot();
						}
						
						if (i % columns < columns - 1)
						{
							ImGui::SameLine();
						}

						i++;
					}
					ImGui::NewLine();
					ImPlot::PopColormap();
				}
				else
				{
					ImGui::BulletText("No Preview Available");
				}
			}
		}

		// Perform tasks
		namespace Workload
		{
			namespace Datasets
			{
				int selectedItem = -1;
				bool loadFlipped = false;
				std::string name;

				void Load()
				{
					if (Tasks::taskProgress.Ready() && selectedItem >= 0)
					{
						std::thread thread(Tasks::LoadDataset, Tasks::Instances::datasetDirs[selectedItem], name, loadFlipped);
						thread.detach();

						name = "";
						selectedItem = -1;
					}
				}

				// Manage Datasets: delete, rename
				namespace Management
				{
					int renameTarget = -1;
					int deleteTarget = -1;

					std::string renameBuffer;
				}

				namespace Viewer
				{
					int viewItem = 0;
					int viewIndex = 0;

					GLuint viewBuffer = NULL;

					void LoadImage(Network::NetworkDataSet* set, int index)
					{
						// clear buffer if texture exists
						if (viewBuffer)
						{
							glDeleteTextures(1, &viewBuffer);
						}

						// data conversion (format)
						float* data = (*set)[index].data;
						int width = set->dataWidth, height = set->dataHeight;

						unsigned char* convertedData = new unsigned char[width * height * 3];

						for (int i = 0; i < width * height * 3; i++)
						{
							convertedData[i] = data[i / 3] * 255;
						}

						glGenTextures(1, &viewBuffer);
						glBindTexture(GL_TEXTURE_2D, viewBuffer);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, convertedData);

						// free memory space
						delete[] convertedData;
					}

					void View()
					{
						if (viewBuffer)
						{
							ImGui::Image((void*)(intptr_t)viewBuffer, ImVec2(300, 300));
						}

						ImGui::SameLine(0, 10);
						ImGui::BeginGroup();
						{
							auto& datasets = Tasks::Instances::datasets;

							// Select Dataset Combobox
							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
							Util::DatasetCombo("Dataset", & viewItem);

							// Index input box
							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
							ImGui::InputInt("Index", &viewIndex);
							Clamp(viewIndex, 0, datasets[viewItem]->Count() - 1);

							// Load image
							if (ImGui::Button(ICON_DOWNLOAD_FILL "Load##dataset_viewer_load", ImVec2(70, 40)))
							{
								LoadImage(datasets[viewItem], viewIndex);
							}
							ImGui::SameLine(); ImGui::Text("Width: %d\nHeight: %d\nLabel: %d", datasets[viewItem]->dataWidth, datasets[viewItem]->dataHeight, (*datasets[viewItem])[viewIndex].label);
						}
						ImGui::EndGroup();
					}
				}
			}
		
			namespace NetworkInstance
			{
				int createOption = 0; // 0=Create new from scratch; 1=Read from file

				namespace Create
				{
					int inNeuronCount = 784;
					int outNeuronCount = 10;
					int hiddenLayerCount = 1;
					int hiddenNeuronCount = 32;

					int activateFunc = 0;

					void CreateNetworkWork()
					{
						if (Tasks::taskProgress.Ready())
						{
							std::thread thread(Tasks::CreateNetwork, inNeuronCount, outNeuronCount, hiddenLayerCount, hiddenNeuronCount, activateFunc);
							thread.detach();
						}
					}
				}

				namespace Read
				{
					void ReadNetworkWork(std::string path)
					{
						if (Tasks::taskProgress.Ready())
						{
							std::thread thread(Tasks::ReadNetwork, path);
							thread.detach();
						}
					}
				}

				namespace Train
				{
					int trainDataset = 0;
					int verifyDataset = 0;

					double learningRate = 0.1;
					double threshold = 0.1;
					
					void TrainNetworkWork()
					{
						if (Tasks::taskProgress.Ready())
						{
							std::thread thread(Tasks::TrainNetwork, learningRate, threshold, trainDataset, verifyDataset, 1);
							thread.detach();
						}
					}
				}

				namespace Verify
				{
					int verifyDataset = 0;

					void VerifyNetworkWork()
					{
						if (Tasks::taskProgress.Ready())
						{
							std::thread thread(Tasks::VerifyNetwork, verifyDataset);
							thread.detach();
						}
					}
				}

				namespace Save
				{
					std::string saveName;

					void SaveNetworkWork()
					{
						std::thread thread(Tasks::SaveNetwork, saveName);
						thread.detach();
					}
				}
			}
		}

		// Popup window code lies here
		namespace PopupWindows
		{
			void DatasetRenamePopupWindow(Components::PopupModal* instance, void* returnPtr)
			{
				namespace renameNamespace = Workload::Datasets::Management;
				std::string& target = renameNamespace::renameBuffer;

				ImGui::InputText("##input_dataset_rename", &target);
				ImGui::Spacing();
				
				// Cancel button
				if (ImGui::Button("Cancel##rename_cancel_button"))
				{
					target = "";
					instance->Close();
				}

				// Submit button
				ImGui::SameLine();
				ImGui::BeginDisabled(target.empty()); // disable submit button if empty
				{
					if (ImGui::Button("Submit##rename_submit_button"))
					{
						if (renameNamespace::renameTarget < 0 || renameNamespace::renameTarget >= Tasks::Instances::datasets.size())
						{
							AddWarning("Invalid Parameter: renameTarget");
						}
						else
						{
							Tasks::Instances::datasets[renameNamespace::renameTarget]->name = target;
						}

						instance->Close();
					}
				}
				ImGui::EndDisabled();
			}

			void DatasetDeletePopupWindow(Components::PopupModal* instance, void* returnPtr)
			{
				int target = Workload::Datasets::Management::deleteTarget;

				ImGui::Text("Are you sure to delete?");
				ImGui::NewLine();
				
				if (ImGui::Button("No"))
				{
					instance->Close();
				}
				ImGui::SameLine(0,10);
				if (ImGui::Button("Yes"))
				{
					Tasks::Instances::RemoveDataset(target);
					instance->Close();
				}
			}
		}

		// Popup instances
		namespace Popups
		{
			Components::PopupModal DatasetRenameModal("Rename", PopupWindows::DatasetRenamePopupWindow, NULL, 200, true);
			Components::PopupModal DatasetDeleteModal("Confirm", PopupWindows::DatasetDeletePopupWindow, NULL, 200);

			Components::PopupDisplayer popupDisplayer({
				&DatasetRenameModal,
				&DatasetDeleteModal
				});
		}

		// Write test code here
		void TestGround()
		{
			/*if (Tasks::Instances::mainNetwork)
			{
				static int num = 0;
				static float range = 1.0;
				if (ImGui::Begin("Heatmap test"))
				{
					ImGui::InputInt("##int", &num);
					ImGui::InputFloat("##float", &range);
					Clamp(num, 0, 63);
					Clamp(range, 0.01f, 1000.0f);

					ImPlot::PushColormap(ImPlotColormap_RdBu);
					if (ImPlot::BeginPlot("Heatmap", ImVec2(500, 500)))
					{
						const ImPlotAxisFlags axes_flags = ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks;

						ImPlot::SetupAxes(NULL, NULL, axes_flags, axes_flags);
						ImPlot::PlotHeatmap("Heatmap", Tasks::Instances::mainNetwork->hiddenLayerList[0][num].weights, 28, 28, -range, range, NULL);
						ImPlot::EndPlot();
					}
					ImGui::SameLine();
					ImPlot::ColormapScale("", -range, range);
					ImPlot::PopColormap();

					ImGui::End();
				}
			}*/
		}

		// Tabbar content
		namespace TabBars
		{
			void SettingsTab()
			{
				// Style options
				ImGui::SeparatorText("Style & Graphics");
				ImGui::Spacing();
				ImGui::BeginGroup();
				{
					const ImVec2 StyleButtonSize(80, 30);
					if (ImGui::Button("Dark", StyleButtonSize)) ImGui::StyleColorsDark(); ImGui::SameLine();
					if (ImGui::Button("Classic", StyleButtonSize)) ImGui::StyleColorsClassic(); ImGui::SameLine();
					if (ImGui::Button("Light", StyleButtonSize)) ImGui::StyleColorsLight(); ImGui::SameLine();
					if (ImGui::Button("Custom 1", StyleButtonSize)) Style::CustomStyle1();
				}
				ImGui::EndGroup();

				ImGui::Spacing();
				ImGui::Checkbox("Vsync Enabled", &UI::windowVsync);

				// Debug options
				ImGui::NewLine(); ImGui::SeparatorText("Debug");
				ImGui::Checkbox("Show Demo Window", &showDemoWindow);
				ImGui::Checkbox("Show Plot Demo Window", &showPlotDemo);

				ImGui::Spacing();
				Util::FrametimeRecorder::RecordFPS();

				if (ImGui::TreeNode("Performance"))
				{
					ImGui::BulletText("Avg Framerate: %.0f FPS", ImGui::GetIO().Framerate);
					Util::FrametimeRecorder::DisplayFPSGraph();

					ImGui::TreePop();
				}

				ImGui::NewLine(); ImGui::SeparatorText("Test Ground");
				TestGround();
			}

			void DatasetTab()
			{
				//==== Load Dataset
				ImGui::SeparatorText("Load");
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				if (ImGui::BeginListBox("##dataset_listbox"))
				{
					int i = 0;
					for (auto& str : Tasks::Instances::datasetDirs)
					{
						const bool isSelected = (i == Workload::Datasets::selectedItem);

						if (ImGui::Selectable(str.c_str(), isSelected))
						{
							Workload::Datasets::selectedItem = isSelected ? -1 : i;
							Workload::Datasets::name = str;
						}

						if (isSelected) ImGui::SetItemDefaultFocus();

						i++;
					}
					ImGui::EndListBox();
				}

				// input dataset name
				ImGui::InputText("Name", &Workload::Datasets::name);

				// refresh dataset list
				if (ImGui::Button(ICON_REFRESH_LINE))
				{
					namespace inst = Tasks::Instances;
					inst::ReadDirList(inst::datasetDirs, inst::datasetPath);
				}

				// if no name is provided or item isn't selected just simply disable start button! 
				ImGui::SameLine();
				ImGui::BeginDisabled(Workload::Datasets::name.empty() || Workload::Datasets::selectedItem == -1);
				if (ImGui::Button(ICON_DOWNLOAD_FILL " Load##load_dataset_button"))
				{
					Workload::Datasets::Load();
				}
				ImGui::EndDisabled();

				// do flip?
				ImGui::SameLine(0, 10);
				ImGui::Checkbox("Flip##dataset_load_flip", &Workload::Datasets::loadFlipped);

				//==== Manage Datasets
				ImGui::NewLine();
				ImGui::SeparatorText("Manage");
				std::vector<Network::NetworkDataSet*>& datasets = Tasks::Instances::datasets;

				if (datasets.size() > 0) {
					if (ImGui::BeginTable("DatasetTable##main_panel_dataset_table", 5, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
					{
						// header
						ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
						ImGui::TableNextColumn(); ImGui::Text("Name");
						ImGui::TableNextColumn(); ImGui::Text("Count");
						ImGui::TableNextColumn(); ImGui::Text("Width");
						ImGui::TableNextColumn(); ImGui::Text("Height");
						ImGui::TableNextColumn(); ImGui::Text("Action");

						for (int i = 0; i < datasets.size(); i++)
						{
							ImGui::TableNextRow();
							ImGui::TableNextColumn(); ImGui::Text("%s", datasets[i]->name.c_str());
							ImGui::TableNextColumn(); ImGui::Text("%d", datasets[i]->Count());
							ImGui::TableNextColumn(); ImGui::Text("%d", datasets[i]->dataWidth);
							ImGui::TableNextColumn(); ImGui::Text("%d", datasets[i]->dataHeight);
							ImGui::TableNextColumn(); 
							{
								if (ImGui::Button("Rename"))
								{
									Workload::Datasets::Management::renameTarget = i;
									Workload::Datasets::Management::renameBuffer = datasets[i]->name;

									Popups::DatasetRenameModal.Open();
								}
								ImGui::SameLine(0, 10);
								if (ImGui::Button("Delete"))
								{
									Workload::Datasets::Management::deleteTarget = i;

									Popups::DatasetDeleteModal.Open();
								}
							}
						}

						ImGui::EndTable();
					}

					ImGui::NewLine();

					//==== View Datasets
					ImGui::SeparatorText("Dataset Viewer");
					ImGui::Spacing();

					Workload::Datasets::Viewer::View();
				}
				else
				{
					ImGui::NewLine();
					ImGui::Indent(); ImGui::Text("No Dataset Loaded");
				}

			}

			void NetworkTab()
			{
				// Create Network
				ImGui::SeparatorText(ICON_ADD_FILL " New Network");
				ImGui::Spacing();

				namespace Create = Workload::NetworkInstance::Create;

				ImGui::BeginGroup();
				{
					ImGui::RadioButton("Create New##network_create_new_radio", &Workload::NetworkInstance::createOption, 0);
					ImGui::SameLine();
					ImGui::RadioButton("Read from archive", &Workload::NetworkInstance::createOption, 1);
				}
				ImGui::EndGroup();

				ImGui::Separator();

				if (Workload::NetworkInstance::createOption == 0) // Create new
				{
					ImGui::InputInt("In-layer Neuron Count", &Create::inNeuronCount);
					ImGui::InputInt("Out-layer Neuron Count", &Create::outNeuronCount);
					ImGui::InputInt("Hidden-layer Count", &Create::hiddenLayerCount);
					ImGui::InputInt("Hidden-layer Neuron Count", &Create::hiddenNeuronCount);
					
					ImGui::Combo("Activate Function", &Create::activateFunc, Network::functionNameList, Network::ActivateFunctionCount, -1);

					Clamp(Create::inNeuronCount, 1, 1048576);
					Clamp(Create::outNeuronCount, 1, 1048576);
					Clamp(Create::hiddenLayerCount, 1, 256);
					Clamp(Create::hiddenNeuronCount, 1, 1048576);

					if (ImGui::Button("Create##network_create_button", ImVec2(70,40)))
					{
						Workload::NetworkInstance::Create::CreateNetworkWork();
					}
				}
				
				if (Workload::NetworkInstance::createOption == 1)
				{
					namespace inst = Tasks::Instances;

					if (ImGui::Button(ICON_REFRESH_LINE))
					{
						inst::ReadFileList(inst::networkFileList, inst::networkPath, { ".network" });
					}

					if (ImGui::BeginTable("Table##network_create_file_list", 3, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg))
					{
						auto& list = inst::networkFileList;

						for (auto& item : list)
						{
							ImGui::TableNextRow();
							ImGui::TableNextColumn(); ImGui::Text("%s", item.name.c_str());
							ImGui::TableNextColumn(); ImGui::Text("%s", FormatFileSize(item.size, 2, SizeFormat::KiloByte).c_str());
							ImGui::TableNextColumn(); 
							if (ImGui::Button(GenID("Load", &item).c_str()))
							{
								Workload::NetworkInstance::Read::ReadNetworkWork(Tasks::Instances::networkPath + item.name);
							}
						}

						ImGui::EndTable();
					}
				}

				auto network = Tasks::Instances::mainNetwork;

				if (network)
				{
					ImGui::NewLine();

					// Train
					if (ImGui::TreeNode("Train##network_train_node"))
					{
						namespace Train = Workload::NetworkInstance::Train;

						Util::DatasetCombo("Train Dataset", &Train::trainDataset);
						Util::DatasetCombo("Verify Dataset", &Train::verifyDataset);

						ImGui::InputDouble("Learning Rate", &Train::learningRate, 0.001, 0.01, "%.6f");
						ImGui::InputDouble("Loss Threshold", &Train::threshold, 0.001, 0.01, "%.6f");

						// Clamp inputs
						Clamp(Train::learningRate, 0.0, 1.0);
						Clamp(Train::threshold, 0.0, 1.0);

						if (ImGui::Button("Train " ICON_FLIGHT_TAKEOFF_FILL, ImVec2(70, 40)))
						{
							Train::TrainNetworkWork();
						}
						ImGui::TreePop();
					}

					// Verify
					if (ImGui::TreeNode("Verify##network_verify_node"))
					{
						namespace Verify = Workload::NetworkInstance::Verify;
						Util::DatasetCombo("VerifyDataset", &Verify::verifyDataset);
						if (ImGui::Button("Verify " ICON_CHECK_DOUBLE_LINE))
						{
							Verify::VerifyNetworkWork();
						}
						ImGui::TreePop();
					}

					if (ImGui::TreeNode("Save##network_save_node"))
					{
						ImGui::InputText("Name", &Workload::NetworkInstance::Save::saveName);
						
						if (ImGui::Button(ICON_SAVE_FILL "Save"))
						{
							Workload::NetworkInstance::Save::SaveNetworkWork();
						}

						ImGui::TreePop();
					}

					if (ImGui::TreeNode("View##network_view_node"))
					{
						if (Tasks::Instances::mainNetwork && !Tasks::Instances::networkOccupied)
						{
							Tasks::Instances::networkOccupied = true; // Occupied the network

							int totalCount = Tasks::Instances::mainNetwork->hiddenLayerCount;
							for (int i = 0; i < totalCount; i++)
							{
								if (ImGui::TreeNode(std::format("Hidden Layer {}", i).c_str()))
								{
									Util::ShowHeatmap(Tasks::Instances::mainNetwork->hiddenLayerList[i]);
									ImGui::TreePop();
								}
							}

							Tasks::Instances::networkOccupied = false;
						}
						ImGui::TreePop();
					}
				}
			}

			void RightPanelAssetTab()
			{
				ImGui::SeparatorText("Datasets");
				ImGui::Spacing();

				std::vector<Network::NetworkDataSet*>& datasets = Tasks::Instances::datasets;
				if (datasets.size() > 0)
				{
					if (ImGui::BeginTable("DatasetTable##right_panel_dataset_table", 4, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
					{
						// header
						ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
						ImGui::TableNextColumn(); ImGui::Text("Name");
						ImGui::TableNextColumn(); ImGui::Text("Count");
						ImGui::TableNextColumn(); ImGui::Text("Width");
						ImGui::TableNextColumn(); ImGui::Text("Height");

						for (int i = 0; i < datasets.size(); i++)
						{
							ImGui::TableNextRow();
							ImGui::TableNextColumn(); ImGui::Text("%s", datasets[i]->name.c_str());
							ImGui::TableNextColumn(); ImGui::Text("%d", datasets[i]->Count());
							ImGui::TableNextColumn(); ImGui::Text("%d", datasets[i]->dataWidth);
							ImGui::TableNextColumn(); ImGui::Text("%d", datasets[i]->dataHeight);
						}

						ImGui::EndTable();
					}
				}
				else
				{
					ImGui::BulletText("No Dataset Loaded");
				}

				if (Tasks::Instances::mainNetwork)
				{
					auto& network = *Tasks::Instances::mainNetwork;
					std::string functionName = Network::functionNameList[(int)network.ActivateFunc];

					ImGui::NewLine();
					ImGui::SeparatorText("Network");
					ImGui::Spacing();

					if (ImGui::BeginTable("NetworkTable##right_panel_network_table", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn(); ImGui::Text("In-Count");
						ImGui::TableNextColumn(); ImGui::Text("%d", network.inNeuronCount);

						ImGui::TableNextRow();
						ImGui::TableNextColumn(); ImGui::Text("Out-Count");
						ImGui::TableNextColumn(); ImGui::Text("%d", network.outNeuronCount);

						ImGui::TableNextRow();
						ImGui::TableNextColumn(); ImGui::Text("Hidden-Count");
						ImGui::TableNextColumn(); ImGui::Text("%d", network.hiddenLayerCount);

						ImGui::TableNextRow();
						ImGui::TableNextColumn(); ImGui::Text("Hidden-Neuron-Count");
						ImGui::TableNextColumn(); ImGui::Text("%d", network.hiddenNeuronCount);

						ImGui::TableNextRow();
						ImGui::TableNextColumn(); ImGui::Text("Activate Function");
						ImGui::TableNextColumn(); ImGui::Text("%s", functionName.c_str());

						ImGui::EndTable();
					}
				}
			}
		}

		int ChildWindowFlags = ImGuiWindowFlags_NoDecoration - ImGuiWindowFlags_NoScrollbar;

		void MainTabBars()
		{
			// settings tab
			if(ImGui::BeginTabItem(ICON_SETTINGS_FILL" Settings##settings_tab", NULL, ImGuiTabItemFlags_None))
			{
				ImGui::BeginChild("##settings_tab_child", ImGui::GetContentRegionAvail(), false, ChildWindowFlags);
				TabBars::SettingsTab();
				ImGui::EndChild();
				ImGui::EndTabItem();
			}

			if(ImGui::BeginTabItem(ICON_DATABASE_2_FILL " Dataset##dataset_tab", NULL, ImGuiTabItemFlags_None))
			{
				ImGui::BeginChild("##dataset_tab_child", ImGui::GetContentRegionAvail(), false, ChildWindowFlags);
				TabBars::DatasetTab();
				ImGui::EndChild();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(ICON_FUNCTION_FILL " Network##network_tab", NULL, ImGuiTabItemFlags_None))
			{
				ImGui::BeginChild("##network_tab_child", ImGui::GetContentRegionAvail(), false, ChildWindowFlags);
				TabBars::NetworkTab();
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
		}

		void RightPanel()
		{
			ImGui::BeginTabBar("Right Panel TabBar##right_tabbar", ImGuiTabBarFlags_AutoSelectNewTabs);

			if (ImGui::BeginTabItem(ICON_INFORMATION_FILL" Assets##assets_tab")) 
			{
				TabBars::RightPanelAssetTab();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(ICON_MESSAGE_FILL" Notifications##notifications_tab", NULL))
			{
				ImGui::BeginChild("##notification_tab_child", ImGui::GetContentRegionAvail(), false, ImGuiWindowFlags_NoDecoration);
				notifyWindow.Render();
				ImGui::EndChild();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		void DrawMainPanel()
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

			const int panelFlag = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus;

			// Left Main panel
			{
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::SetNextWindowSize(ImVec2(windowWidth - rightPanelWidth, windowHeight - bottomBarHeight));

				ImGui::Begin("MainWindow##main_window", NULL, panelFlag);

				ImGui::BeginTabBar("MainTab##main_tab", ImGuiTabBarFlags_None);

				MainTabBars();

				ImGui::EndTabBar();

				ImGui::End();
			}

			// Bottom status bar
			{
				ImGui::SetNextWindowPos(ImVec2(0, windowHeight - bottomBarHeight));
				ImGui::SetNextWindowSize(ImVec2(windowWidth, bottomBarHeight));

				ImGui::Begin("Bottom Bar##bottom_bar", NULL, panelFlag);

				// status text
				const char* statusIcons[] = { ICON_ZZZ_FILL, ICON_PLAY_FILL, ICON_CHECK_DOUBLE_FILL, ICON_ERROR_WARNING_FILL };
				ImGui::Text("%s %s   ", statusIcons[(int)Tasks::taskProgress.state], Tasks::taskProgress.display.c_str());

				// progress bar
				ImGui::SameLine();

				if (Tasks::taskProgress.state == Tasks::TaskState::Working)
					if (Tasks::taskProgress.isIndeterminate)
					{
						ImGui::IndeterminateProgressBar(ImVec2(250, 20));
					}
					else
					{
						ImGui::ProgressBar(Tasks::taskProgress.progress, ImVec2(250, 20));
					}


				ImGui::End();
			}

			// Right panel
			{
				ImGui::SetNextWindowPos(ImVec2(windowWidth, 0), 0, ImVec2(1, 0));
				ImGui::SetNextWindowSize(ImVec2(rightPanelWidth, windowHeight - bottomBarHeight));

				ImGui::Begin("Right Panel##right_panel", NULL, panelFlag);

				RightPanel();

				ImGui::End();
			}

			ImGui::PopStyleVar(1);
		}
	}

	void UpdateTaskProgressNotification()
	{
		if (Tasks::taskProgress.state == Tasks::TaskState::Done)
		{
			notifyQueue.Queue(Components::Notification(Tasks::taskProgress.display, ICON_CHECK_DOUBLE_FILL" Success", 0));
			Tasks::taskProgress.state = Tasks::TaskState::Idle;
			Tasks::taskProgress.Idle();
		}

		if (Tasks::taskProgress.state == Tasks::TaskState::Error)
		{
			notifyQueue.Queue(Components::Notification(Tasks::taskProgress.display, ICON_ERROR_WARNING_FILL" Error", 0));
			Tasks::taskProgress.Idle();
		}
	}

	void DrawLayout()
	{
		UpdateTaskProgressNotification();

		Layout::DrawMainPanel();
		Layout::Popups::popupDisplayer.Display();

		if(Layout::showDemoWindow)
			ImGui::ShowDemoWindow();

		if(Layout::showPlotDemo)
			ImPlot::ShowDemoWindow();
	}
}