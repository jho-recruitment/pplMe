#     File:  generate_pplme_dataset.py
#   Author:  j.ho
#  Purpose:  Generates a pplMe dataset of vaguely real-looking people by
#            consulting various online sources and using their data as a
#            basis for randomization.


from __future__ import print_function
import csv
import datetime
import math
import os
import random
import sys
import urllib
import uuid


# Ideally 100000000, but on my rubbishy laptop, this takes about 6 minutes.  &:/
DEFAULT_NUMBER_OF_USERS = 10000000


# Number of Internet users by country.
COUNTRY_INTERNET_USERS_DATA_SOURCE = "https://www.cia.gov/library/publications/the-world-factbook/rankorder/rawdata_2153.txt"

# Area of country by country.
COUNTRY_AREA_DATA_SOURCE = "https://www.cia.gov/library/publications/the-world-factbook/rankorder/rawdata_2147.txt"

# Country "location" by country.
COUNTRY_LOCATION_DATA_SOURCE = "http://www.opengeocode.org/cude/download.php?file=/home/fashions/public_html/opengeocode.org/download/cow.txt"
COUNTRY_NAME_FIELDNUM = 21
COUNTRY_LATITUDE_FIELDNUM = 61
COUNTRY_LONGITUDE_FIELDNUM = 62

# A list of popular surnames.
LASTNAMES_DATA_SOURCE = "http://www.quietaffiliate.com/Files/CSV_Database_of_Last_Names.csv"

# A list of popular given names.
FIRSTNAMES_DATA_SOURCE = "http://www.quietaffiliate.com/Files/CSV_Database_of_First_Names.csv"


# Grabs the data from the specified row, then processes it as CSV, one row
# at a time, with a bit of help from the specified row-processor.
def process_csv_data_source(data_source_name,
                            data_source_uri,
                            row_processor,
                            field_delim=","):
    class StatusOkRequiringURLopener(urllib.FancyURLopener):
        def http_error_default(self, url, fp, errcode, errmsg, headers):
            print("Failed to download {0} from {1}: {2}: {3}".format(
                    data_source_name,
                    data_source_uri,
                    errcode,
                    errmsg))
            sys.exit(1)
    urllib._urlopener = StatusOkRequiringURLopener()
    try:
        tempfilename, _ = urllib.urlretrieve(data_source_uri)
        try:
            with open(tempfilename, "rU") as csvfile:
                csvreader = csv.reader(csvfile,
                                       delimiter=field_delim,
                                       skipinitialspace=True)
                for row in csvreader:
                    try:
                        row_processor(row)
                    except Exception as e:
                        print("Failed to process {0} row: {1} [{2}]".format(  \
                                data_source_name,
                                e,
                                row));
                        sys.exit(1)
        finally:
            os.remove(tempfilename)
    except Exception as e:
        print("Failed to download {0}: {1}".format(data_source_name, e),
              file=sys.stderr)
        sys.exit(1)


# Convenience function to deal with thousands separator.
def parse_number(s):
    return int(s.replace(",", ""))


# Grab the number of Internet users by country.
def slurp_country_internet_users_data():
    internet_users_data = dict()
    def populate_internet_users_data(row):
        internet_users_data[row[1]] = parse_number(row[2])
    process_csv_data_source("Country Internet Users Data",
                            COUNTRY_INTERNET_USERS_DATA_SOURCE,
                            populate_internet_users_data,
                            "\t")
    return internet_users_data


# Grab the country area data.
def slurp_country_area_data():
    country_area_data = dict()
    def populate_country_area_data(row):
        country_area_data[row[1]] = parse_number(row[2])
    process_csv_data_source("Country Area Data",
                            COUNTRY_AREA_DATA_SOURCE,
                            populate_country_area_data,
                            "\t")
    return country_area_data


# Grab the country location data.
def slurp_country_location_data():
    country_location_data = dict()
    def populate_country_location_data(row):
        # Skip (possibly BOM-prefixed) comment lines.
        if not row[0].startswith("#")  \
                and not row[0].startswith("\xef\xbb\xbf#"):
            latitude = row[COUNTRY_LATITUDE_FIELDNUM]
            longitude = row[COUNTRY_LONGITUDE_FIELDNUM]
            # Skip column headers.
            if not latitude == "latitude" and not longitude == "longitude":
                country_location_data[row[COUNTRY_NAME_FIELDNUM]] =  \
                    float(latitude), float(longitude)
    process_csv_data_source("Country Location Data",
                            COUNTRY_LOCATION_DATA_SOURCE,
                            populate_country_location_data,
                            ";")
    return country_location_data


# Grab the lastnames data.
def slurp_lastnames_data():
    lastnames_data = list()
    def populate_lastnames_data(row):
        if row[0] != "lastname":
            lastnames_data.append(row[0])
    process_csv_data_source("Lastnames Data",
                            LASTNAMES_DATA_SOURCE,
                            populate_lastnames_data)
    return lastnames_data


# Grab the firstnames data.
def slurp_firstnames_data():
    firstnames_data = list()
    def populate_firstnames_data(row):
        if row[0] != "firstname":
            firstnames_data.append(row[0])
    process_csv_data_source("Firstnames Data",
                            FIRSTNAMES_DATA_SOURCE,
                            populate_firstnames_data)
    return firstnames_data


# Generate a location in a country's "area".
def generate_random_location(country_location_data, country_area_data, country):
    latitude = country_location_data[country][0]
    longitude = country_location_data[country][1]
    # If we have area data, then pick a random position in the country,
    # assuming that its area is a perfect square.  O&;D
    if country in country_area_data:
        country_length = math.sqrt(country_area_data[country])
        # Latitude only slightly varies so to make things easy assume constant.
        latitude += random.uniform(-country_length / 2, country_length / 2)  \
            / 111
        # And we also cheat by assuming that Terra is a perfect sphere of
        # radius 6378137m [thanks Wikipedia / WGS84].  And by assuming that
        # the length of longitude is constant up and down the country.
        longitudinal_length = math.acos(
            180 * (country_length / 2) / math.pi / 6378137)
        longitude += random.uniform(-longitudinal_length, longitudinal_length)
    return latitude, longitude


# Do the Gubbins.
def generate_pplme_dataset(number_of_users):
    internet_users_data = slurp_country_internet_users_data()
    country_area_data = slurp_country_area_data()
    country_location_data = slurp_country_location_data()
    lastnames_data = slurp_lastnames_data()
    firstnames_data = slurp_firstnames_data()

    # Here we calculate the sum total number of Internet Users for all
    # countries.
    total_users = reduce(lambda total, users: total + users,
                         internet_users_data.itervalues(),
                         0)

    # Iterate across each country, and generate a relative proportion of random
    # users for that country.
    for country, users in internet_users_data.iteritems():
        weighted_users = (float(users) / total_users) * number_of_users
        if country in country_location_data:
            for user in xrange(0, int(weighted_users)):
                id = uuid.uuid4()
                # Pick a random firstname and lastname and concatenate.
                firstname = firstnames_data[
                    random.randrange(0, len(firstnames_data) - 1)]
                lastname = lastnames_data[
                    random.randrange(0, len(lastnames_data) - 1)]
                name = firstname + " " + lastname
                # Generate a random age between 18 and 100 (relative to today).
                dob = datetime.date.today() +  \
                    datetime.timedelta(random.randrange(-100, -18) * 365)
                latitude, longitude = generate_random_location(
                    country_location_data, country_area_data, country)
                print("{0},{1},{2},{3},{4}".
                      format(id, name, dob, latitude, longitude))

    return True


if __name__ == "__main__":
    if len(sys.argv) == 1 or len(sys.argv) == 2:
        success = generate_pplme_dataset(  
            int(sys.argv[1]) if len(sys.argv) == 2 else DEFAULT_NUMBER_OF_USERS)
        sys.exit(0 if success else 1)
    else:
        print("Usage: {0} [<number of users>]")
        sys.exit(2)
