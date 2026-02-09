import { useState, useEffect } from 'react';
import axios from 'axios';
import LandingPage from './pages/LandingPage';
import VisualizationPage from './pages/VisualizationPage';
import Header from './components/Header';
import FloorTabs from './components/FloorTabs';
import ParkingGrid from './components/ParkingGrid';
import Analytics from './components/Analytics';
import ParkModal from './components/ParkModal';
import ExitModal from './components/ExitModal';
import RecommendationBanner from './components/RecommendationBanner';
import './App.css';

const API_BASE = '/api';

function App() {
  const [parkingData, setParkingData] = useState(null);
  const [analyticsData, setAnalyticsData] = useState(null);
  const [currentFloor, setCurrentFloor] = useState(0);
  const [loading, setLoading] = useState(true);
  const [lastUpdated, setLastUpdated] = useState(new Date());
  const [currentPage, setCurrentPage] = useState('landing');
  
  // Modal states
  const [parkModalOpen, setParkModalOpen] = useState(false);
  const [exitModalOpen, setExitModalOpen] = useState(false);
  const [selectedSlot, setSelectedSlot] = useState(null);
  const [recommendedSlot, setRecommendedSlot] = useState(null);

  // Fetch data from C backend
  const fetchData = async () => {
    try {
      const [stateRes, analyticsRes] = await Promise.all([
        axios.get(`${API_BASE}/state`),
        axios.get(`${API_BASE}/analytics`)
      ]);
      
      setParkingData(stateRes.data);
      setAnalyticsData(analyticsRes.data);
      setLastUpdated(new Date());
      setLoading(false);
      
      // Auto switch to available floor if current floor is full
      if (stateRes.data && currentPage === 'park') {
        checkFloorAvailability(stateRes.data);
      }
    } catch (error) {
      console.error('Error fetching data:', error);
      setLoading(false);
    }
  };

  // Auto-refresh every 3 seconds
  useEffect(() => {
    fetchData();
    const interval = setInterval(fetchData, 3000);
    return () => clearInterval(interval);
  }, []);

  // ✅ FIX: Get recommendation when entering park mode, changing floor, or data changes
  useEffect(() => {
    if (currentPage === 'park' && parkingData) {
      // Add delay to ensure fresh data
      const timer = setTimeout(() => {
        getRecommendation();
      }, 500);
      
      return () => clearTimeout(timer);
    } else {
      // Clear recommendation when leaving park page
      setRecommendedSlot(null);
    }
  }, [currentPage, currentFloor, parkingData?.total_occupied]); // ✅ Add total_occupied dependency

  // Refresh data when returning to landing page
  useEffect(() => {
    if (currentPage === 'landing') {
      fetchData();
    }
  }, [currentPage]);

  // Check floor availability and auto-switch
  const checkFloorAvailability = (data) => {
    const floor = data.floors[currentFloor];
    const availableSlots = floor.total_slots - floor.occupied_slots;
    
    if (availableSlots === 0) {
      // Find first available floor
      for (let i = 0; i < data.floors.length; i++) {
        const f = data.floors[i];
        if (f.total_slots - f.occupied_slots > 0) {
          setCurrentFloor(i);
          break;
        }
      }
    }
  };

  // ✅ FIXED: Get recommendation with fresh data validation
  const getRecommendation = async () => {
    if (!parkingData) return;
    
    try {
      console.log('🔍 Getting recommendation for floor:', currentFloor);
      
      // Include current floor in recommendation request
      const response = await axios.get(`${API_BASE}/recommend?vehicle_type=1&floor=${currentFloor}`);
      
      console.log('📊 Recommendation response:', response.data);
      
      if (response.data.status === 'success' && response.data.recommendations.length > 0) {
        const bestRec = response.data.recommendations[0];
        
        // ✅ FIX: Refresh parking data first to ensure slot is still available
        const stateRes = await axios.get(`${API_BASE}/state`);
        const floor = stateRes.data.floors[bestRec.floor];
        const slot = floor.slots.find(s => s.id === bestRec.slot_id);
        
        if (slot && !slot.is_occupied) {
          setRecommendedSlot({
            ...slot,
            bfs_distance: bestRec.entry_distance,
            score: bestRec.score,
            floor: bestRec.floor
          });
          console.log('✅ Recommended slot set:', bestRec.slot_id);
        } else {
          console.log('❌ Recommended slot not available:', bestRec.slot_id);
          setRecommendedSlot(null);
        }
      } else {
        console.log('❌ No recommendations available');
        setRecommendedSlot(null);
      }
    } catch (error) {
      console.error('❌ Error getting recommendation:', error);
      setRecommendedSlot(null);
    }
  };

  // Handle action selection from landing page
  const handleActionSelect = (action) => {
    setCurrentPage(action);
    if (action === 'park') {
      getRecommendation();
    }
  };

  // Handle slot click
  const handleSlotClick = (slot) => {
    setSelectedSlot(slot);
    if (currentPage === 'park' && !slot.is_occupied) {
      setParkModalOpen(true);
    } else if (currentPage === 'exit' && slot.is_occupied) {
      setExitModalOpen(true);
    }
  };

  // ✅ FIXED: Park vehicle with proper state updates
  const handleParkVehicle = async (vehicleData) => {
    try {
      console.log('🚗 Parking vehicle:', vehicleData);
      
      const response = await axios.post(`${API_BASE}/park`, vehicleData);
      
      if (response.data.status === 'success') {
        alert(`✓ Vehicle parked successfully!\n\nSlot: ${response.data.slot}\nFloor: ${response.data.floor}\nVehicle: ${response.data.vehicle_number}\nDistance: ${response.data.path_length} steps`);
        
        // ✅ Clear recommendation first
        setRecommendedSlot(null);
        setParkModalOpen(false);
        
        // ✅ Fetch latest data and wait for completion
        await fetchData();
        
        // ✅ Get new recommendation after data refresh (longer delay)
        if (currentPage === 'park') {
          setTimeout(() => {
            console.log('🔄 Fetching new recommendation after parking...');
            getRecommendation();
          }, 1000);
        }
      } else {
        alert(`✗ ${response.data.message}`);
        setParkModalOpen(false);
      }
    } catch (error) {
      console.error('❌ Parking error:', error.response?.data || error.message);
      alert('Error parking vehicle: ' + (error.response?.data?.message || error.message));
      setParkModalOpen(false);
    }
  };

  // ✅ FIXED: Exit vehicle with proper state updates
  const handleExitVehicle = async () => {
    try {
      const response = await axios.post(`${API_BASE}/exit`, {
        slot_id: selectedSlot.id
      });
      
      if (response.data.status === 'success') {
        alert(`✓ Vehicle exited successfully!\n\nVehicle: ${response.data.vehicle}\nSlot: ${response.data.slot}`);
        
        setExitModalOpen(false);
        
        // ✅ Fetch latest data and wait for completion
        await fetchData();
        
        // ✅ Get new recommendation if in park mode
        if (currentPage === 'park') {
          setTimeout(() => {
            console.log('🔄 Fetching new recommendation after exit...');
            getRecommendation();
          }, 1000);
        }
      } else {
        alert(`✗ ${response.data.message}`);
        setExitModalOpen(false);
      }
    } catch (error) {
      alert('Error exiting vehicle: ' + error.message);
      setExitModalOpen(false);
    }
  };

  if (loading) {
    return (
      <div className="loading-screen">
        <div className="loader"></div>
        <p>Connecting to parking system...</p>
      </div>
    );
  }

  if (!parkingData) {
    return (
      <div className="error-screen">
        <h2>⚠️ Cannot connect to server</h2>
        <p>Please make sure the C program (parking_system.exe) is running</p>
        <button onClick={fetchData} className="retry-btn">🔄 Retry</button>
      </div>
    );
  }

  // Landing page
  if (currentPage === 'landing') {
    return <LandingPage onSelectAction={handleActionSelect} parkingData={parkingData} />;
  }

  // Visualization page
  if (currentPage === 'visualize') {
    return <VisualizationPage parkingData={parkingData} onBack={() => setCurrentPage('landing')} />;
  }

  // Park/Exit pages (main interface)
  return (
    <div className="app">
      <Header data={parkingData} lastUpdated={lastUpdated} onBackToHome={() => setCurrentPage('landing')} />
      
      {currentPage === 'park' && recommendedSlot && (
        <RecommendationBanner 
          slot={recommendedSlot} 
          onAccept={() => {
            setSelectedSlot(recommendedSlot);
            setParkModalOpen(true);
          }}
          onDismiss={() => setRecommendedSlot(null)}
        />
      )}
      
      <main className="main-content">
        <div className="parking-section">
          <FloorTabs
            floors={parkingData.floors}
            currentFloor={currentFloor}
            onFloorChange={(idx) => {
              setCurrentFloor(idx);
              setRecommendedSlot(null);
            }}
            mode={currentPage}
          />
          
          <ParkingGrid
            floor={parkingData.floors[currentFloor]}
            onSlotClick={handleSlotClick}
            recommendedSlot={recommendedSlot?.id}
            mode={currentPage}
          />
        </div>
        
        <Analytics data={analyticsData} />
      </main>

      {/* Modals */}
      <ParkModal
        isOpen={parkModalOpen}
        onClose={() => setParkModalOpen(false)}
        onSubmit={handleParkVehicle}
        slot={selectedSlot}
      />

      <ExitModal
        isOpen={exitModalOpen}
        onClose={() => setExitModalOpen(false)}
        onConfirm={handleExitVehicle}
        slot={selectedSlot}
      />
    </div>
  );
}

export default App;
