include_directories(${anttrackingUNIL_SOURCE_DIR}/inc)

add_library(atrkutil SHARED exception.cpp utils.cpp datfile.cpp tags3.cpp)

add_executable(change_tagid change_tagid.cpp)
target_link_libraries(change_tagid atrkutil)

add_executable(controldat controldat.cpp)
target_link_libraries(controldat atrkutil)

add_executable(define_death define_death.cpp)
target_link_libraries(define_death atrkutil)

add_executable(filter_interactions_cut_immobile filter_interactions_cut_immobile.cpp)
target_link_libraries(filter_interactions_cut_immobile atrkutil)

add_executable(filter_interactions_no_cut filter_interactions_no_cut.cpp)
target_link_libraries(filter_interactions_no_cut atrkutil)

add_executable(heatmap3_tofile heatmap3_tofile.cpp histogram.cpp statistics.cpp)
target_link_libraries(heatmap3_tofile atrkutil)

add_executable(interaction interaction_tags3_corrected.cpp)
target_link_libraries(interaction atrkutil)

# The following 3 interaction prorgams are versions specific to the Science 2018 paper.
# The one above is the standard version.
add_executable(interaction_all_close_contacts interaction_all_close_contacts.cpp)
target_link_libraries(interaction_all_close_contacts atrkutil)

add_executable(interaction_any_overlap interaction_any_overlap.cpp)
target_link_libraries(interaction_any_overlap atrkutil)

add_executable(interaction_close_front_contacts interaction_close_front_contacts.cpp)
target_link_libraries(interaction_close_front_contacts atrkutil)

add_executable(time_investment time_investment.cpp plume.cpp)
target_link_libraries(time_investment atrkutil)

add_executable(trackconverter trackconverter_modular.cpp trackconverter_functions.cpp)
target_link_libraries(trackconverter atrkutil)

add_executable(trajectory trajectory.cpp)
target_link_libraries(trajectory atrkutil)

add_executable(zone_converter zone_converter.cpp plume.cpp)
target_link_libraries(zone_converter atrkutil)

add_executable(extrapolate_step1 extrapolate_coordinates_step1.cpp)
target_link_libraries(extrapolate_step1 atrkutil)

add_executable(extrapolate_step2 extrapolate_coordinates_step2.cpp)
target_link_libraries(extrapolate_step2 atrkutil)

install(TARGETS atrkutil LIBRARY DESTINATION lib)
