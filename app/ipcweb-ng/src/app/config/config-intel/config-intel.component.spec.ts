import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ConfigIntelComponent } from './config-intel.component';

describe('ConfigIntelComponent', () => {
  let component: ConfigIntelComponent;
  let fixture: ComponentFixture<ConfigIntelComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ConfigIntelComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ConfigIntelComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
