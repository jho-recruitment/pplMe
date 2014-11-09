/**
 *  @file
 *  @brief   Implementation for pplme::engine::PplSlurper.
 *  @author  j.ho
 */


#include "ppl_slurper.h"
#include <fstream>
#include <locale>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/tokenizer.hpp>
#include <boost/uuid/string_generator.hpp>
#include <glog/logging.h>
#include "libpplmecore/person.h"
#include "ppl_repository.h"


namespace pplme {
namespace engine {


PplSlurper::PplSlurper(std::string const& ppl_data_csv_file) :
    ppl_data_csv_file_{ppl_data_csv_file} {}


bool PplSlurper::Populate(PplRepository* ppl_repo) {
  std::ifstream csv_file{ppl_data_csv_file_};

  // Ugh naked new.  &:(
  // &:'(
  auto dif = new boost::gregorian::date_input_facet{};
  dif->set_iso_extended_format();
  csv_file.imbue(std::locale{std::locale::classic(), dif});
  
  std::string id_string;
  std::string name;

  int line_num = 0;
  char const* error = nullptr;
  while (csv_file) {
    ++line_num;

    // Read id.
    if (!std::getline(csv_file, id_string, ',')) {
      if (!csv_file.eof())
        error = "failed to read id";
      break;
    }
    core::PersonId id;
    try {
      id = core::PersonId{boost::uuids::string_generator()(id_string)};
    } catch (std::runtime_error const&) {
      error = "failed to parse id as UUID";
      break;
    }

    // Read name.
    std::string name;
    if (!std::getline(csv_file, name, ',')) {
      error = "failed to read name";
      break;
    }

    // Read date-of-birth.
    boost::gregorian::date dob;
    if (!(csv_file >> dob)) {
      error = "failed to read date-of-birth";
      break;
    }
    if (csv_file.get() != ',') {
      error = "missing comma after date-of-birth";
      break;
    }

    // Read latitude.
    float latitude;
    if (!(csv_file >> latitude)) {
      error = "failed to read latitude";
      break;
    }
    if (csv_file.get() != ',') {
      error = "missing comma after latitude";
      break;
    }

    // Read longitude.
    float longitude;
    if (!(csv_file >> longitude)) {
      error = "failed to read longitude";
      break;
    }

    // Make sure there's nothing trailing on the line.
    if (csv_file.get() != '\n') {
      error = "unexected stuffs at end of line";
      break;
    }
    
    std::unique_ptr<core::Person> person{new core::Person{
      id,
      name,
      dob,
      core::GeoPosition{
          core::GeoPosition::DecimalLatitude{latitude},
          core::GeoPosition::DecimalLongitude{longitude}}}};
    ppl_repo->AddPerson(std::move(person));       
  }

  if (error)
    LOG(ERROR) << ppl_data_csv_file_ << ":" << line_num << ": " << error;
  
  return !error && csv_file.eof();
}


}  // namespace engine
}  // namespace pplme
